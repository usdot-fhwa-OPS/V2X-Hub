using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.IO;
using Microsoft.Maps.MapControl.WPF;
using Microsoft.Win32;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Diagnostics;
using System.ComponentModel;
using SharpKml.Base;
using SharpKml.Dom;
using SharpKml.Engine;
using System.Text.RegularExpressions;
using System.Collections.Concurrent;

namespace V2IHubSimulator
{
    public enum RunThreadType
    {
        SIMULATION = 1,
        KML = 2
    }
    public class RunThreadData
    {
        public RunThreadType Type { get; set; }
        public int VehicleID { get; set; }
        public int CloneNumber { get; set; }
    }
        /// <summary>
        /// Interaction logic for MainWindow.xaml
        /// </summary>
        public partial class MainWindow : Window
    {
        private object _UIDataLock = new object();
        private string _simulatorAddress;
        private int _simulatorPort;
        private double _resolutionMeters;
        private int _bsmFrequenceMS = 100;
        private bool? _loop = true;
        private double _kmlStartSliderValue = 0;
        private double _kmlEndSliderValue = 100;

        private List<Thread> _runThreadList = new List<Thread>();
        private volatile int _runningThreadCount = 0;
        private volatile bool  _stopThreads = false;
        private bool _exitApplication = false;
        private int _currentVehicleID = 0;
        private int _maxVehicles = 10;
        private Color[] _colors = new Color[10] { Colors.Cyan, Colors.Coral, Colors.Pink, Colors.Gold, Colors.Lavender, Colors.Lime, Colors.Orange, Colors.Magenta, Colors.PowderBlue, Colors.Yellow };
        private Brush _defaultLabelBrush;
        private bool _useResolution = false;
        private MapLayer _pinLayer = new MapLayer();
        private MapLayer _segmentLayer = new MapLayer();
        private MapLayer _vehicleLayer = new MapLayer();
        private MapLayer _kmlPinLayer = new MapLayer();
        private MapLayer _kmlPathLayer = new MapLayer();
        private ConcurrentDictionary<int, MapPolygon> _vehiclePolygons = new ConcurrentDictionary<int, MapPolygon>();
        private ConcurrentDictionary<int, Brush> _vehicleBrushes = new ConcurrentDictionary<int, Brush>();
        private Brush _alertBrush = new SolidColorBrush(Colors.Red);
        private KmlFile _kmlFile;
        private ConcurrentDictionary<int, List<KMLWaypoint>> _kmlWaypoints = new ConcurrentDictionary<int, List<KMLWaypoint>>();
        private volatile int _kmlLastWaypointVehicleID = 0;
        private volatile int _kmlLastWaypointIndex = 0;
        private long _kmlMinTick = 0;
        private long _kmlMaxTick = 0;
        private bool _sendSRM = false;
        private int _vehicleRoleForSRM = 14; // 12 = police, 13 = fire, 14 = ambulance
        private int _bsmPerSRM = 10; //determine frequency of SRMs, 10 = 1 per second


        private bool MapInEditMode { get; set; }

        private Simulation CurrentSimulation { get; set; }

        public MainWindow()
        {
            InitializeComponent();

            myMap.MouseDoubleClick += MyMap_MouseDoubleClick;

            MapInEditMode = false;

            SimulatorAddressTextBox.Text = "10.30.100.50";
            //SimulatorPortTextBox.Text = "48917";  // simulatedBSM port
            SimulatorPortTextBox.Text = "26789";  // MessageReceiver port
            ResolutionTextBox.Text = "100";

            CurrentSimulation = new Simulation();
            _defaultLabelBrush = VehicleColorLabel.Background;
            myMap.Children.Add(_pinLayer);
            myMap.Children.Add(_segmentLayer);
            myMap.Children.Add(_vehicleLayer);
            myMap.Children.Add(_kmlPinLayer);
            myMap.Children.Add(_kmlPathLayer);
            Microsoft.Maps.MapControl.WPF.Location loc = new Microsoft.Maps.MapControl.WPF.Location(39.099639, -94.578555);
            myMap.Center = loc;
            myMap.ZoomLevel = 4;

            KMLStartTimeSlider.Value = 0;
            KMLEndTimeSlider.Value = 100;
            KMLStartTimeSlider.SmallChange = KMLStartTimeSlider.SmallChange / 10.0;
            KMLEndTimeSlider.SmallChange = KMLEndTimeSlider.SmallChange / 10.0;
        }

        ~MainWindow()
        {
            //foreach (Thread t in _runThreadList)
            //    if (t.IsAlive)
            //        t.Abort();
        }

        void MainWindow_Closing(object sender, CancelEventArgs e)
        {
            if (_runningThreadCount > 0)
            {
                _exitApplication = true;
                _stopThreads = true;
                e.Cancel = true;
            }
            if (_runningThreadCount == 0)
                e.Cancel = false;
        }

        private void SaveUIData()
        {
            lock(_UIDataLock)
            {
                _simulatorAddress = SimulatorAddressTextBox.Text;
                _simulatorPort = Convert.ToInt32(SimulatorPortTextBox.Text);
                _resolutionMeters = Convert.ToDouble(ResolutionTextBox.Text);
                _bsmFrequenceMS = Convert.ToInt32(ResolutionTextBox.Text);
                _loop = LoopCheckBox.IsChecked;
                _kmlStartSliderValue = KMLStartTimeSlider.Value;
                _kmlEndSliderValue = KMLEndTimeSlider.Value;
            }
        }
        
        private void IncrementRunningThreadCount()
        {
            _runningThreadCount++;
            if (_runningThreadCount == 1)
            {
                DisableUI();
                Image img = new Image();
                img.Source = new BitmapImage(new Uri("Resources/stop.png", UriKind.Relative));
                PlayPauseButton.Content = img;
                PlayPauseButton.ToolTip = "Stop Simulation";
            }
        }

        private void DecrementRunningThreadCount()
        {
            _runningThreadCount--;
            if (_runningThreadCount == 0)
            {
                if (_exitApplication)
                {
                    Application.Current.Shutdown();
                }
                else
                {
                    Image img = new Image();
                    img.Source = new BitmapImage(new Uri("Resources/play.png", UriKind.Relative));
                    PlayPauseButton.Content = img;
                    PlayPauseButton.ToolTip = "Run Simulation";
                    _vehicleLayer.Children.Clear();
                    _vehiclePolygons.Clear();
                    _vehicleBrushes.Clear();
                    EnableUI();
                }
            }
        }

        private void DisableUI()
        {
            ScenarioNameTextBox.IsEnabled = false;
            SimulatorAddressTextBox.IsEnabled = false;
            SimulatorPortTextBox.IsEnabled = false;
            ResolutionTextBox.IsEnabled = false;
            AddVehicleButton.IsEnabled = false;
            DeleteVehicleButton.IsEnabled = false;
            VehicleIDComboBox.IsEnabled = false;
            WaypointSpeedTextBox.IsEnabled = false;
            WaypointPauseTextBox.IsEnabled = false;
            NewButton.IsEnabled = false;
            OpenButton.IsEnabled = false;
            SaveButton.IsEnabled = false;
            EditModeButton.IsEnabled = false;
            KMLStartTimeSlider.IsEnabled = false;
            KMLStartTimeSliderText.IsEnabled = false;
            KMLEndTimeSlider.IsEnabled = false;
            KMLEndTimeSliderText.IsEnabled = false;
            KMLFindVehicleButton.IsEnabled = true;
        }

        private void EnableUI()
        {
            ScenarioNameTextBox.IsEnabled = true;
            SimulatorAddressTextBox.IsEnabled = true;
            SimulatorPortTextBox.IsEnabled = true;
            ResolutionTextBox.IsEnabled = true;
            AddVehicleButton.IsEnabled = true;
            DeleteVehicleButton.IsEnabled = true;
            VehicleIDComboBox.IsEnabled = true;
            WaypointSpeedTextBox.IsEnabled = true;
            WaypointPauseTextBox.IsEnabled = true;
            NewButton.IsEnabled = true;
            OpenButton.IsEnabled = true;
            SaveButton.IsEnabled = true;
            EditModeButton.IsEnabled = true;
            KMLStartTimeSlider.IsEnabled = true;
            KMLStartTimeSliderText.IsEnabled = true;
            KMLEndTimeSlider.IsEnabled = true;
            KMLEndTimeSliderText.IsEnabled = true;
            KMLFindVehicleButton.IsEnabled = false;
        }

        private void MyMap_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            e.Handled = true;
            if (MapInEditMode)
            {
                int currentWPNum = CurrentSimulation.WaypointLists[_currentVehicleID].Count;

                Microsoft.Maps.MapControl.WPF.Location pushPinLocation = myMap.ViewportPointToLocation(e.GetPosition(myMap));
                Console.WriteLine("Location: " + pushPinLocation.Latitude + ", " + pushPinLocation.Longitude);

                SimulationWaypoint wp = new SimulationWaypoint();
                wp.Latitude = pushPinLocation.Latitude;
                wp.Longitude = pushPinLocation.Longitude;
                wp.Speed_mph = 25;
                wp.Pause_seconds = 0.0;
                wp.Background = new SolidColorBrush(CurrentSimulation.Colors[_currentVehicleID]);
                wp.WaypointNumber = currentWPNum;
                wp.VehicleID = _currentVehicleID;


                SimulationWaypointPushPin pin = wp.GetPushPin();

                pin.Location = pushPinLocation;
                pin.MouseDown += Pin_MouseDown;
                _pinLayer.Children.Add(pin);

                if (currentWPNum > 0)
                {
                    SimulationWaypointSegment seg = wp.GetPolyline(CurrentSimulation.WaypointLists[_currentVehicleID][currentWPNum - 1].Latitude, CurrentSimulation.WaypointLists[_currentVehicleID][currentWPNum - 1].Longitude);
                    _segmentLayer.Children.Add(seg);
                }


                CurrentSimulation.WaypointLists[_currentVehicleID].Add(wp);

            }
        }

        private void Pin_MouseDown(object sender, MouseButtonEventArgs e)
        {
            SimulationWaypointPushPin pin = sender as SimulationWaypointPushPin;

            SimulationWaypoint wp = pin.ParentWaypoint;
            if (wp.VehicleID != _currentVehicleID)
                VehicleIDComboBox.SelectedItem = wp.VehicleID.ToString();
            WaypointLatLabel.Content = wp.Latitude.ToString();
            WaypointLonLabel.Content = wp.Longitude.ToString();
            WaypointSpeedTextBox.Text = wp.Speed_mph.ToString();
            WaypointPauseTextBox.Text = wp.Pause_seconds.ToString();
            WaypointNumberLabel.Content = (wp.WaypointNumber + 1).ToString();
        }

        private void SearchLocationTextBox_KeyUp(object sender, KeyEventArgs e)
        {
            if(e.Key == Key.Enter)
            {
                Search(SearchLocationTextBox.Text);
            }
        }

        private void Search(string text)
        {
            try
            {
                Microsoft.Maps.MapControl.WPF.Location loc = BingMapHelper.SearchBingMaps(text);

                myMap.Center = loc;
                myMap.ZoomLevel = 15;

            }catch(Exception ex)
            {

            }
        }

        private void ClearSimulation()
        {
            _pinLayer.Children.Clear();
            _segmentLayer.Children.Clear();
            _vehicleLayer.Children.Clear();

            VehicleIDComboBox.Items.Clear();
            VehicleIDComboBox.SelectedIndex = -1;
            if (MapInEditMode)
                ToggleEditMode();
            //clear waypoint
            ClearWaypoint();
            CurrentSimulation = new Simulation();
        }

        private void LoadSimulationToMap(Simulation simulation)
        {
            List<SimulationWaypoint> waypointList;
            SimulationWaypoint lastWaypoint = new SimulationWaypoint();

            ClearWaypoint();
            VehicleIDComboBox.SelectedIndex = -1;
            CurrentSimulation = simulation;

            ScenarioNameTextBox.Text = simulation.Name;
            foreach (var key in CurrentSimulation.WaypointLists.Keys)
            {
                waypointList = CurrentSimulation.WaypointLists[key];
                foreach (var waypoint in waypointList)
                {
                    Pushpin pin = waypoint.GetPushPin();
                    pin.MouseDown += Pin_MouseDown;
                    _pinLayer.Children.Add(pin);
                    if (waypoint.WaypointNumber > 0)
                    {
                        SimulationWaypointSegment seg = waypoint.GetPolyline(lastWaypoint.Latitude, lastWaypoint.Longitude);
                        _segmentLayer.Children.Add(seg);
                    }
                    lastWaypoint = waypoint;
                }
                VehicleIDComboBox.Items.Add(key.ToString());
            }
            if (VehicleIDComboBox.Items.Count > 0)
                VehicleIDComboBox.SelectedIndex = 0;

        }

        #region "Button Handlers"

        private void SearchButton_Click(object sender, RoutedEventArgs e)
        {
            Search(SearchLocationTextBox.Text);
        }

        private void EditModeButton_Click(object sender, RoutedEventArgs e)
        {
            ToggleEditMode();
        }

        private void ToggleEditMode()
        {
            if (!MapInEditMode)
            {
                if (VehicleIDComboBox.SelectedIndex == -1)
                    return;
                MapInEditMode = true;
                EditModeButton.Content = FindResource("MapStopEdit");
                EditModeButton.ToolTip = "Stop Adding Waypoints";
            }
            else
            {
                MapInEditMode = false;
                EditModeButton.Content = FindResource("MapEdit");
                EditModeButton.ToolTip = "Add Waypoints";
            }
        }

        private void NewButton_Click(object sender, RoutedEventArgs e)
        {
            ClearSimulation();
        }

       
        private void OpenButton_Click(object sender, RoutedEventArgs e)
        {
            FileStream fs;
            Match m;
            KMLWaypoint kwp;
            int vehicleId;
            SharpKml.Dom.Point point;
            SharpKml.Dom.Timestamp timestamp;
            bool firstPoint = true;
            try
            {
                OpenFileDialog openFileDialog = new OpenFileDialog();
                openFileDialog.Multiselect = false;
                openFileDialog.Filter = "Scenario and KML files (*.json,*.kml)|*.json;*.kml|Scenario files (*.json)|*.json|KML files (*.kml)|*.kml|All files (*.*)|*.*";
                openFileDialog.InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments);
                if (openFileDialog.ShowDialog() == true)
                {
                    string filename = openFileDialog.FileName;
                    if (filename.EndsWith(".kml"))
                    {
                        fs = File.OpenRead(filename);
                        _kmlFile = KmlFile.Load(fs);
                        Kml kml = _kmlFile.Root as Kml;
                        if (kml != null)
                        {
                            _kmlWaypoints.Clear();
                            foreach (var placemark in kml.Flatten().OfType<Placemark>())
                            {
                                if (placemark.Description != null && placemark.Geometry != null && placemark.Time != null)
                                {
                                    m = Regex.Match(placemark.Description.Text, @"VehicleId: (?<vehicleId>\d+)");
                                    if (m.Success)
                                    {
                                        vehicleId = Int32.Parse(m.Groups["vehicleId"].Value);
                                        if (!_kmlWaypoints.Keys.Contains(vehicleId))
                                            _kmlWaypoints[vehicleId] = new List<KMLWaypoint>();
                                        kwp = new KMLWaypoint();
                                        m = Regex.Match(placemark.Description.Text, @"Heading: (?<heading>\d+\.\d+)");
                                        if (m.Success)
                                            kwp.Heading = Double.Parse(m.Groups["heading"].Value);
                                        else
                                            kwp.Heading = 0;
                                        m = Regex.Match(placemark.Name, @"Speed: (?<speed>\d+\.\d+) MPH");
                                        if (m.Success)
                                            kwp.Speed_mph = Double.Parse(m.Groups["speed"].Value);
                                        else
                                            kwp.Speed_mph = 0;
                                        point = (SharpKml.Dom.Point)placemark.Geometry;
                                        kwp.Latitude = point.Coordinate.Latitude;
                                        kwp.Longitude = point.Coordinate.Longitude;
                                        timestamp = (SharpKml.Dom.Timestamp)placemark.Time;
                                        kwp.Tick = timestamp.When.Value.Ticks;
                                        if (firstPoint)
                                        {
                                            _kmlMinTick = kwp.Tick;
                                            _kmlMaxTick = kwp.Tick;
                                            firstPoint = false;
                                        }
                                        else
                                        {
                                            if (kwp.Tick < _kmlMinTick)
                                                _kmlMinTick = kwp.Tick;
                                            if (kwp.Tick > _kmlMaxTick)
                                                _kmlMaxTick = kwp.Tick;
                                        }
                                        _kmlWaypoints[vehicleId].Add(kwp);
                                    }
                                }
                                
                            }

                            //set kml min max time
                            KMLStartTimeSliderText.Text = new DateTime(_kmlMinTick).ToString();
                            KMLEndTimeSliderText.Text = new DateTime(_kmlMaxTick).ToString();

                            KMLRefreshPath();
                        }
                    }
                    else
                        LoadSimulationToMap(Simulation.SimulationFromFile(filename));

                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error opening file: " + ex.Message);
            }
        }

        private void KMLRefreshPath()
        {

            //foreach (var coordinate in linestring.Coordinates)
            //{
            //    Pushpin pp = new Pushpin();
            //    pp.Location = new Microsoft.Maps.MapControl.WPF.Location(coordinate.Latitude, coordinate.Longitude);
            //    //pp.Content = 1;
            //    pp.Background = new SolidColorBrush(Colors.Red); ;
            //    pp.Foreground = new SolidColorBrush(Colors.Black);
            //    _kmlPinLayer.Children.Add(pp);
            //}
            
            long startTick = _kmlMinTick + (long)(KMLStartTimeSlider.Value / 100.0 * (_kmlMaxTick - _kmlMinTick));
            long endTick = _kmlMinTick + (long)(KMLEndTimeSlider.Value / 100.0 * (_kmlMaxTick - _kmlMinTick));
            _kmlPathLayer.Children.Clear();
            if (startTick >= endTick)
            {
                MessageBox.Show("KML Start Time must be less then KML End Time.", "Run Error", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }
            bool haveFirst = false;
            foreach (var kwpList in _kmlWaypoints.Values)
            {
                MapPolyline mpl = new MapPolyline();
                mpl.Stroke = new SolidColorBrush(Colors.Red);
                mpl.StrokeThickness = 2;
                LocationCollection locs = new LocationCollection();
                KMLWaypoint lastWP = null;
                foreach (var wp in kwpList)
                {
                    if (wp.Tick >= startTick && wp.Tick <= endTick)
                    {
                        locs.Add(new Microsoft.Maps.MapControl.WPF.Location(wp.Latitude, wp.Longitude));
                        lastWP = wp;
                        if (!haveFirst)
                        {
                            haveFirst = true;
                            Pushpin pp = new Pushpin();
                            pp.Location = new Microsoft.Maps.MapControl.WPF.Location(wp.Latitude, wp.Longitude);
                            pp.Background = new SolidColorBrush(Colors.Green);
                            pp.Foreground = new SolidColorBrush(Colors.Black);
                            _kmlPathLayer.Children.Add(pp);
                        }
                    }
                }
                mpl.Locations = locs;
                _kmlPathLayer.Children.Add(mpl);
                if (lastWP != null)
                {
                    Pushpin pp = new Pushpin();
                    pp.Location = new Microsoft.Maps.MapControl.WPF.Location(lastWP.Latitude, lastWP.Longitude);
                    pp.Background = new SolidColorBrush(Colors.Red);
                    pp.Foreground = new SolidColorBrush(Colors.Black);
                    _kmlPathLayer.Children.Add(pp);
                }
            }
        }

        private void SaveButton_Click(object sender, RoutedEventArgs e)
        {
            SaveFileDialog saveFileDialog = new SaveFileDialog();
            saveFileDialog.Filter = "Scenario files (*.json)|*.json";
            saveFileDialog.InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments);
            saveFileDialog.FileName = ScenarioNameTextBox.Text;
            if (saveFileDialog.ShowDialog() == true)
            {
                string filename = saveFileDialog.FileName;

                Simulation.SerializeToFile(CurrentSimulation, filename);
            }

        }

        private void PlayPauseButton_Click(object sender, RoutedEventArgs e)
        {
            Thread runThread;
            int i;

            if (_runningThreadCount == 0)
            {
                _stopThreads = false;
                if (CurrentSimulation.WaypointLists.Count == 0 && _kmlWaypoints.Count == 0)
                    return;
                if (MapInEditMode)
                    ToggleEditMode();
                _runThreadList.Clear();
                foreach (int key in CurrentSimulation.WaypointLists.Keys)
                {
                    for (i = 0; i < CurrentSimulation.Clones[key] + 1; i++)
                    {
                        RunThreadData data = new RunThreadData();
                        data.Type = RunThreadType.SIMULATION;
                        data.VehicleID = key;
                        data.CloneNumber = i;
                        runThread = new Thread(new ParameterizedThreadStart(Run));
                        runThread.Start(data);
                        _runThreadList.Add(runThread);
                    }
                }
                foreach (int key in _kmlWaypoints.Keys)
                {
                    RunThreadData data = new RunThreadData();
                    data.Type = RunThreadType.KML;
                    data.VehicleID = key;
                    data.CloneNumber = 0;
                    runThread = new Thread(new ParameterizedThreadStart(Run));
                    runThread.Start(data);
                    _runThreadList.Add(runThread);
                }
            }
            else
            {
                //stop threads
                _stopThreads = true;
            }
        }

        private void SRMButton_Click(object sender, RoutedEventArgs e)
        {
            if (_sendSRM)
            {
                Image img = new Image();
                img.Source = new BitmapImage(new Uri("Resources/srm-send.png", UriKind.Relative));
                SRMButton.Content = img;
                SRMButton.ToolTip = "Send SRM";
                _sendSRM = false;
            }
            else
            {
                Image img = new Image();
                img.Source = new BitmapImage(new Uri("Resources/srm-stop.png", UriKind.Relative));
                SRMButton.Content = img;
                SRMButton.ToolTip = "Stop Sending SRM";
                _sendSRM = true;
            }
        }
            
        #endregion

        private void ScenarioNameTextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            e.Handled = true;

            CurrentSimulation.Name = ScenarioNameTextBox.Text;
        }

        private void WaypointSpeedTextBox_KeyUp(object sender, KeyEventArgs e)
        {
            e.Handled = true;
           string newText = WaypointSpeedTextBox.Text;

            int selectedWpNumber = Int32.Parse(WaypointNumberLabel.Content as string);
            try
            {
                double newSpeed;
                Double.TryParse(newText, out newSpeed);

                CurrentSimulation.SetWaypointSpeed(_currentVehicleID, selectedWpNumber - 1, newSpeed);

            }
            catch (Exception ex)
            {

            }
        }

        private void WaypointPauseTextBox_KeyUp(object sender, KeyEventArgs e)
        {
            e.Handled = true;
            string newText = WaypointPauseTextBox.Text;

            int selectedWpNumber = Int32.Parse(WaypointNumberLabel.Content as string);
            try
            {
                double newPause;
                Double.TryParse(newText, out newPause);

                CurrentSimulation.SetWaypointPause(_currentVehicleID, selectedWpNumber - 1, newPause);

            }
            catch (Exception ex)
            {

            }
        }
        
        private void Run(object data)
        {
            try
            {
                int i, j;
                SimulationWaypoint p1 = new SimulationWaypoint();
                SimulationWaypoint p2 = new SimulationWaypoint();
                WGS84Point wp1;
                WGS84Point wp2;
                double distance;
                double heading;
                double speed1MetersPerSec;
                double speed2MetersPerSec;
                int numSegments;
                double segmentLength;
                int sleepTimeMS;
                WGS84Point segmentPoint;
                RunThreadData threadData = (RunThreadData)data;
                int currentVehicleID = threadData.VehicleID;
                int cloneNumber = threadData.CloneNumber;
                Stopwatch stopwatch = new Stopwatch();
                long firstBSMTimeMS = 0;
                long segmentStartTimeMS = 0;
                long nextBSMTimeMS = 0;
                long nextSegmentBSMTimeMS = 0;
                long pointCount = 0;
                bool done = false;
                double accelerationMetersPerSecSquared;
                double nextDistance = 0.0;
                double nextSpeedMetersPerSec;
                long segmentDurationMS;
                long startTick = 0;
                int startIndex = 0;
                long endTick = 0;
                int endIndex = 0;
                bool haveStartIndex = false;
                bool haveEndIndex = false;
                int bsmSentSinceLastSRM;
                UdpClient myUdpClient = null;
                string simulatorAddress;
                int simulatorPort;
                double resolutionMeters;
                int bsmFrequenceMS;
                bool? loop;
                double kmlStartSliderValue;
                double kmlEndSliderValue;


                if (threadData.Type == RunThreadType.SIMULATION)
                    if (CurrentSimulation.WaypointLists[currentVehicleID].Count < 2)
                        return;

                if (threadData.Type == RunThreadType.KML)
                    if (_kmlWaypoints[currentVehicleID].Count < 2)
                        return;
                
                this.Dispatcher.Invoke(() => IncrementRunningThreadCount());

                this.Dispatcher.Invoke(() => SaveUIData());

                lock (_UIDataLock)
                {
                    simulatorAddress = _simulatorAddress;
                    simulatorPort = _simulatorPort;
                    resolutionMeters = _resolutionMeters;
                    bsmFrequenceMS = _bsmFrequenceMS;
                    loop = _loop;
                    kmlStartSliderValue = _kmlStartSliderValue;
                    kmlEndSliderValue = _kmlEndSliderValue;
                }

                myUdpClient = new UdpClient(simulatorAddress, simulatorPort);

                if (kmlStartSliderValue >= kmlEndSliderValue)
                {
                    MessageBox.Show("KML Start Time must be less then KML End Time.", "Run Error", MessageBoxButton.OK, MessageBoxImage.Error);
                    this.Dispatcher.Invoke(() => DecrementRunningThreadCount());
                    return;
                }

                loop = true;
                if (threadData.Type == RunThreadType.SIMULATION)
                    if (cloneNumber > 0)
                    Thread.Sleep((int)(CurrentSimulation.CloneOffsets[currentVehicleID] * 1000 * cloneNumber));
                while (loop == true && !_stopThreads)
                {
                    stopwatch.Start();

                    if (threadData.Type == RunThreadType.SIMULATION)
                    {
                        pointCount = 0;
                        bsmSentSinceLastSRM = 10; //set to 10 at start of first segment to trigger first SRM send
                        for (i = 0; i < CurrentSimulation.WaypointLists[currentVehicleID].Count && !_stopThreads; i++)
                        {
                            p1 = p2;
                            p2 = CurrentSimulation.WaypointLists[currentVehicleID][i];
                            if (i != 0)
                            {
                                wp1 = new WGS84Point(p1.Latitude, p1.Longitude);
                                wp2 = new WGS84Point(p2.Latitude, p2.Longitude);
                                //get total distance
                                distance = GeoVector.DistanceInMeters(wp1, wp2);
                                //get heading
                                heading = GeoVector.BearingInDegrees(wp1, wp2);
                                //get speeds
                                speed1MetersPerSec = p1.Speed_mph * 0.44704;
                                speed2MetersPerSec = p2.Speed_mph * 0.44704;
                                //get acceleration
                                accelerationMetersPerSecSquared = ((speed2MetersPerSec * speed2MetersPerSec) - (speed1MetersPerSec * speed1MetersPerSec)) / (distance * 2.0);
                                //get time spent on this segment
                                if (speed1MetersPerSec == speed2MetersPerSec)
                                    segmentDurationMS = (long)((distance / speed1MetersPerSec) * 1000.0);
                                else
                                    segmentDurationMS = (long)(((speed2MetersPerSec - speed1MetersPerSec) / accelerationMetersPerSecSquared) * 1000.0);
                                if (speed2MetersPerSec == 0.0 && p2.Pause_seconds > 0.0)
                                    segmentDurationMS += (long)(p2.Pause_seconds * 1000.0);
                                if (_useResolution) // the old way, dont use
                                {
                                    //use resolution
                                    //get number of points
                                    numSegments = (int)(distance / resolutionMeters);
                                    if (numSegments == 0)
                                        numSegments = 1;
                                    //get segment length
                                    segmentLength = distance / numSegments;
                                    //get sleep time
                                    sleepTimeMS = (int)(distance / speed1MetersPerSec / numSegments * 1000);
                                    for (j = 0; j <= numSegments && !_stopThreads; j++)
                                    {
                                        if (j == 0)
                                        {
                                            //first point, send only if its the first segment
                                            if (i == 1)
                                                if (!SendBSM(ref myUdpClient, simulatorAddress, simulatorPort, threadData.Type, currentVehicleID + cloneNumber, heading, speed1MetersPerSec, p1.Latitude, p1.Longitude, 0.0))
                                                    j = numSegments + 1;
                                            Thread.Sleep(sleepTimeMS);
                                        }
                                        else if (j == numSegments)
                                        {
                                            //last point
                                            if (!SendBSM(ref myUdpClient, simulatorAddress, simulatorPort, threadData.Type, currentVehicleID + cloneNumber, heading, speed1MetersPerSec, p2.Latitude, p2.Longitude, 0.0))
                                                j = numSegments + 1;
                                        }
                                        else
                                        {
                                            //calculate point
                                            segmentPoint = GeoVector.DestinationPoint(wp1, heading, j * segmentLength);
                                            if (!SendBSM(ref myUdpClient, simulatorAddress, simulatorPort, threadData.Type, currentVehicleID + cloneNumber, heading, speed1MetersPerSec, segmentPoint.Latitude, segmentPoint.Longitude, 0.0))
                                                j = numSegments + 1;
                                            Thread.Sleep(sleepTimeMS);
                                        }
                                    }
                                }
                                else //the new way, always use
                                {

                                    //use frequency
                                    done = false;
                                    while (!done && !_stopThreads)
                                    {
                                        if (pointCount == 0 && i == 1)
                                        {
                                            //first point of first segment
                                            //pause if set on first point
                                            if (p1.Pause_seconds > 0.0)
                                                Thread.Sleep((int)(p1.Pause_seconds * 1000.0));
                                            //send BSM
                                            if (_sendSRM && bsmSentSinceLastSRM >= _bsmPerSRM)
                                            {
                                                //also send SRM
                                                SendBSM(ref myUdpClient, simulatorAddress, simulatorPort, threadData.Type, currentVehicleID + cloneNumber, heading, speed1MetersPerSec, p1.Latitude, p1.Longitude, 0.0, true);
                                                SendSRM(ref myUdpClient, simulatorAddress, simulatorPort, threadData.Type, currentVehicleID + cloneNumber, heading, speed1MetersPerSec, p1.Latitude, p1.Longitude, _vehicleRoleForSRM);
                                                bsmSentSinceLastSRM = 0;
                                            }
                                            else
                                            {
                                                SendBSM(ref myUdpClient, simulatorAddress, simulatorPort, threadData.Type, currentVehicleID + cloneNumber, heading, speed1MetersPerSec, p1.Latitude, p1.Longitude, 0.0);
                                                bsmSentSinceLastSRM++;
                                            }
                                            firstBSMTimeMS = stopwatch.ElapsedMilliseconds;
                                            segmentStartTimeMS = firstBSMTimeMS;
                                        }

                                        //get next bsm time
                                        nextBSMTimeMS = firstBSMTimeMS + ((pointCount + 1) * bsmFrequenceMS);
                                        //get distance along segment at next BSM
                                        nextSegmentBSMTimeMS = nextBSMTimeMS - segmentStartTimeMS;
                                        if (nextSegmentBSMTimeMS <= segmentDurationMS)
                                        {
                                            //next point is on segment
                                            //calculate speed
                                            nextSpeedMetersPerSec = speed1MetersPerSec + ((nextSegmentBSMTimeMS / 1000.0) * accelerationMetersPerSecSquared);
                                            if (nextSpeedMetersPerSec < 0.0)
                                            {
                                                //going backwards not implemented, we stop at segment end
                                                segmentPoint = wp2;
                                            }
                                            else
                                            {
                                                //still moving forward or stopped
                                                //calculate position
                                                nextDistance = speed1MetersPerSec * (nextSegmentBSMTimeMS / 1000.0) + ((accelerationMetersPerSecSquared * (nextSegmentBSMTimeMS / 1000.0) * (nextSegmentBSMTimeMS / 1000.0)) / 2.0);
                                                segmentPoint = GeoVector.DestinationPoint(wp1, heading, nextDistance);
                                            }
                                            //calculate sleep time and sleep
                                            sleepTimeMS = (int)(nextBSMTimeMS - stopwatch.ElapsedMilliseconds);
                                            if (sleepTimeMS > 0)
                                                Thread.Sleep(sleepTimeMS);
                                            //send BSM and possibly SRM
                                            if (_sendSRM)
                                            {
                                                if (bsmSentSinceLastSRM >= _bsmPerSRM)
                                                {
                                                    //also send SRM, vehicle id likely not same as in BSM in real world
                                                    SendBSM(ref myUdpClient, simulatorAddress, simulatorPort, threadData.Type, currentVehicleID + cloneNumber, heading, nextSpeedMetersPerSec, segmentPoint.Latitude, segmentPoint.Longitude, 0.0, true);
                                                    SendSRM(ref myUdpClient, simulatorAddress, simulatorPort, threadData.Type, currentVehicleID + cloneNumber + 1000, heading, nextSpeedMetersPerSec, segmentPoint.Latitude, segmentPoint.Longitude, _vehicleRoleForSRM);
                                                    bsmSentSinceLastSRM = 0;
                                                }
                                                else
                                                {
                                                    //also send SRM, vehicle id likely not same as in BSM in real world
                                                    SendBSM(ref myUdpClient, simulatorAddress, simulatorPort, threadData.Type, currentVehicleID + cloneNumber, heading, nextSpeedMetersPerSec, segmentPoint.Latitude, segmentPoint.Longitude, 0.0);
                                                    SendSRM(ref myUdpClient, simulatorAddress, simulatorPort, threadData.Type, currentVehicleID + cloneNumber + 1000, heading, nextSpeedMetersPerSec, segmentPoint.Latitude, segmentPoint.Longitude, _vehicleRoleForSRM);
                                                    bsmSentSinceLastSRM++;
                                                }
                                            }
                                            else
                                            {
                                                SendBSM(ref myUdpClient, simulatorAddress, simulatorPort, threadData.Type, currentVehicleID + cloneNumber, heading, nextSpeedMetersPerSec, segmentPoint.Latitude, segmentPoint.Longitude, 0.0);
                                                bsmSentSinceLastSRM++;
                                            }
                                            //increment point count
                                            pointCount++;
                                        }
                                        else
                                        {
                                            //next point is on next segment
                                            //calculate next segment start time
                                            segmentStartTimeMS = segmentStartTimeMS + segmentDurationMS;
                                            //exit this segment loop
                                            done = true;
                                        }

                                    }


                                }
                            }
                        }
                    }
                    else if (threadData.Type == RunThreadType.KML)
                    {

                        startTick = _kmlMinTick + (long)(kmlStartSliderValue / 100.0 * (_kmlMaxTick - _kmlMinTick));
                        endTick = _kmlMinTick + (long)(kmlEndSliderValue / 100.0 * (_kmlMaxTick - _kmlMinTick));
                        //get start and end indexes
                        startIndex = 0;
                        endIndex = _kmlWaypoints[currentVehicleID].Count - 1;
                        haveStartIndex = false;
                        haveEndIndex = false;
                        for (i = 0; i < _kmlWaypoints[currentVehicleID].Count && (!haveStartIndex || !haveEndIndex); i++)
                        {
                            if (!haveStartIndex && _kmlWaypoints[currentVehicleID][i].Tick >= startTick)
                            {
                                startIndex = i;
                                haveStartIndex = true;
                            }
                            if (!haveEndIndex && _kmlWaypoints[currentVehicleID][i].Tick > endTick)
                            {
                                endIndex = i - 1;
                                haveEndIndex = true;
                            }
                        }

                        if (!haveStartIndex || startIndex >= endIndex)
                        {
                            this.Dispatcher.Invoke(() => DecrementRunningThreadCount());
                            return;
                        }
                        for (i = startIndex; i <= endIndex && !_stopThreads; i++)
                        {
                            if (i == startIndex)
                            {
                                //first point
                                SendBSM(ref myUdpClient, simulatorAddress, simulatorPort, threadData.Type, currentVehicleID, _kmlWaypoints[currentVehicleID][i].Heading, _kmlWaypoints[currentVehicleID][i].Speed_mph * 0.44704, _kmlWaypoints[currentVehicleID][i].Latitude, _kmlWaypoints[currentVehicleID][i].Longitude, 0.0);
                                startTick = _kmlWaypoints[currentVehicleID][i].Tick;
                                firstBSMTimeMS = stopwatch.ElapsedMilliseconds;
                                _kmlLastWaypointVehicleID = currentVehicleID;
                                _kmlLastWaypointIndex = i;
                                i++;
                            }
                            //calculate sleep time
                            sleepTimeMS = (int)(((_kmlWaypoints[currentVehicleID][i].Tick - startTick) / 10000) - (stopwatch.ElapsedMilliseconds - firstBSMTimeMS));
                            if (sleepTimeMS > 0)
                                Thread.Sleep(sleepTimeMS);
                            SendBSM(ref myUdpClient, simulatorAddress, simulatorPort, threadData.Type, currentVehicleID, _kmlWaypoints[currentVehicleID][i].Heading, _kmlWaypoints[currentVehicleID][i].Speed_mph * 0.44704, _kmlWaypoints[currentVehicleID][i].Latitude, _kmlWaypoints[currentVehicleID][i].Longitude, 0.0);
                            _kmlLastWaypointVehicleID = currentVehicleID;
                            _kmlLastWaypointIndex = i;

                        }
                    }

                    
                    this.Dispatcher.Invoke(() => SaveUIData());
                    stopwatch.Stop();

                    //sleep before starting next loop
                    if (threadData.Type == RunThreadType.SIMULATION)
                    {
                        if (loop == true && !_stopThreads && !_useResolution)
                            Thread.Sleep(bsmFrequenceMS);
                    }
                    else if (threadData.Type == RunThreadType.KML)
                    {
                        //sleep 1 millisecond for KML
                        Thread.Sleep(1);
                    }
                }
            }
            catch (Exception ex)
            {
            }
            this.Dispatcher.Invoke(() => DecrementRunningThreadCount());
        }

        bool SendBSM(ref UdpClient client, string simulatorAddress, int simulatorPort, RunThreadType type, int vehicleId, double heading, double speed,
            double latitude, double longitude, double elevation, bool alert = false)
        {
            byte[] buffer = new byte[1024];
            int length;
            short svalue;
            int lvalue;
            try
            {
                if (client == null)
                    client = new UdpClient(simulatorAddress, simulatorPort);

                //HEADER
                //message type
                svalue = 1000;
                (BitConverter.GetBytes(IPAddress.HostToNetworkOrder(svalue))).CopyTo(buffer, 0);
                //version
                svalue = 1;
                (BitConverter.GetBytes(IPAddress.HostToNetworkOrder(svalue))).CopyTo(buffer, 2);
                //source id
                svalue = 55;
                (BitConverter.GetBytes(IPAddress.HostToNetworkOrder(svalue))).CopyTo(buffer, 4);
                //body length
                svalue = 24;
                (BitConverter.GetBytes(IPAddress.HostToNetworkOrder(svalue))).CopyTo(buffer, 6);

                //BODY
                //vehicle id
                lvalue = vehicleId;
                (BitConverter.GetBytes(IPAddress.HostToNetworkOrder(lvalue))).CopyTo(buffer, 8);
                //heading 0 to 359.9875 degrees
                lvalue = (int)(heading * 1000000);
                (BitConverter.GetBytes(IPAddress.HostToNetworkOrder(lvalue))).CopyTo(buffer, 12);
                //speed in meters per second
                lvalue = (int)(speed * 1000);
                (BitConverter.GetBytes(IPAddress.HostToNetworkOrder(lvalue))).CopyTo(buffer, 16);
                //latitude
                lvalue = (int)((latitude + 180) * 1000000);
                (BitConverter.GetBytes(IPAddress.HostToNetworkOrder(lvalue))).CopyTo(buffer, 20);
                //longitude
                lvalue = (int)((longitude + 180) * 1000000);
                (BitConverter.GetBytes(IPAddress.HostToNetworkOrder(lvalue))).CopyTo(buffer, 24);
                //elevation -409.5 to 6143.9 meters
                lvalue = (int)((elevation + 500) * 1000);
                (BitConverter.GetBytes(IPAddress.HostToNetworkOrder(lvalue))).CopyTo(buffer, 28);

                length = 32;
                client.Send(buffer, length);

                this.Dispatcher.Invoke(() => DrawVehicle(type, vehicleId, heading, speed, latitude, longitude, alert));
            }
            catch (Exception ex)
            {
                Console.WriteLine("Exception in SendBSM: " + ex.Message);
                try
                {
                    if (client != null)
                        client.Close();
                }
                catch
                {
                }
                client = null;
                return false;
            }

            return true;
        }

        bool SendSRM(ref UdpClient client, string simulatorAddress, int simulatorPort, RunThreadType type, int vehicleId, double heading, double speed,
            double latitude, double longitude, int role)
        {
            byte[] buffer = new byte[1024];
            int length;
            short svalue;
            int lvalue;
            try
            {
                if (client == null)
                    client = new UdpClient(simulatorAddress, simulatorPort);

                //HEADER
                //message type
                svalue = 2000;
                (BitConverter.GetBytes(IPAddress.HostToNetworkOrder(svalue))).CopyTo(buffer, 0);
                //version
                svalue = 1;
                (BitConverter.GetBytes(IPAddress.HostToNetworkOrder(svalue))).CopyTo(buffer, 2);
                //source id
                svalue = 55;
                (BitConverter.GetBytes(IPAddress.HostToNetworkOrder(svalue))).CopyTo(buffer, 4);
                //body length
                svalue = 24;
                (BitConverter.GetBytes(IPAddress.HostToNetworkOrder(svalue))).CopyTo(buffer, 6);

                //BODY
                //vehicle id
                lvalue = vehicleId;
                (BitConverter.GetBytes(IPAddress.HostToNetworkOrder(lvalue))).CopyTo(buffer, 8);
                //heading 0 to 359.9875 degrees
                lvalue = (int)(heading * 1000000);
                (BitConverter.GetBytes(IPAddress.HostToNetworkOrder(lvalue))).CopyTo(buffer, 12);
                //speed in meters per second
                lvalue = (int)(speed * 1000);
                (BitConverter.GetBytes(IPAddress.HostToNetworkOrder(lvalue))).CopyTo(buffer, 16);
                //latitude
                lvalue = (int)((latitude + 180) * 1000000);
                (BitConverter.GetBytes(IPAddress.HostToNetworkOrder(lvalue))).CopyTo(buffer, 20);
                //longitude
                lvalue = (int)((longitude + 180) * 1000000);
                (BitConverter.GetBytes(IPAddress.HostToNetworkOrder(lvalue))).CopyTo(buffer, 24);
                //role
                lvalue = role;
                (BitConverter.GetBytes(IPAddress.HostToNetworkOrder(lvalue))).CopyTo(buffer, 28);

                length = 32;
                client.Send(buffer, length);
                
            }
            catch (Exception ex)
            {
                Console.WriteLine("Exception in SendSRM: " + ex.Message);
                try
                {
                    if (client != null)
                        client.Close();
                }
                catch
                {
                }
                client = null;
                return false;
            }

            return true;
        }

        private void VehicleIDComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (VehicleIDComboBox.SelectedIndex != -1)
            {
                _currentVehicleID = Convert.ToInt32(VehicleIDComboBox.SelectedItem.ToString());
                VehicleColorLabel.Background = new SolidColorBrush(CurrentSimulation.Colors[_currentVehicleID]);
                //clear waypoint
                ClearWaypoint();
            }
            else
            {
                VehicleColorLabel.Background = _defaultLabelBrush;
            }
        }

        private void AddVehicleButton_Click(object sender, RoutedEventArgs e)
        {
            if (VehicleIDComboBox.Items.Count == _maxVehicles)
                return;
            NewVehicleDialog dialog = new NewVehicleDialog();
            int vehicleID;
            int numberOfClones = 0;
            double cloneOffset = 0.0;
            if (dialog.ShowDialog() == true)
            {
                if (int.TryParse(dialog.VehicleID, out vehicleID))
                {
                    if (dialog.NumberOfClones != "")
                    {
                        if (!int.TryParse(dialog.NumberOfClones, out numberOfClones))
                            return;
                        if (numberOfClones < 0 || numberOfClones > 50)
                            return;
                        if (!double.TryParse(dialog.CloneOffset, out cloneOffset))
                            return;
                        if (cloneOffset < 0.0 || cloneOffset > 600.0)
                            return;
                    }
                    if (VehicleIDComboBox.Items.Contains(dialog.VehicleID))
                        return;
                    CurrentSimulation.WaypointLists[vehicleID] = new List<SimulationWaypoint>();
                    foreach(Color c in _colors)
                    {
                        if (!CurrentSimulation.Colors.Values.Contains(c))
                        {
                            CurrentSimulation.Colors[vehicleID] = c;
                            for (int i = 1; i <= numberOfClones;i++)
                                CurrentSimulation.Colors[vehicleID + i] = c;
                            break;
                        }
                    }

                    CurrentSimulation.Clones[vehicleID] = numberOfClones;
                    CurrentSimulation.CloneOffsets[vehicleID] = cloneOffset;

                    VehicleIDComboBox.Items.Add(dialog.VehicleID);
                    VehicleIDComboBox.SelectedItem = dialog.VehicleID;
                    //clear waypoint
                    ClearWaypoint();
                }
            }
        }

        private void ClearWaypoint()
        {
            //clear waypoint
            WaypointLatLabel.Content = "";
            WaypointLonLabel.Content = "";
            WaypointSpeedTextBox.Text = "";
            WaypointPauseTextBox.Text = "";
            WaypointNumberLabel.Content = "";
        }

        private void DeleteVehicleButton_Click(object sender, RoutedEventArgs e)
        {
            SimulationWaypointPushPin pp;
            SimulationWaypointSegment pl;
            List<Object> oList = new List<Object>();
            if (VehicleIDComboBox.SelectedIndex != -1)
            {
                foreach (Object child in _pinLayer.Children)
                {
                    if (child is SimulationWaypointPushPin)
                    {
                        pp = (SimulationWaypointPushPin)child;
                        if (pp.ParentWaypoint.VehicleID == _currentVehicleID)
                            oList.Add(pp);
                    }
                }
                foreach (Object o in oList)
                    _pinLayer.Children.Remove((UIElement)o);
                oList.Clear();
                foreach (Object child in _segmentLayer.Children)
                {
                    if (child is SimulationWaypointSegment)
                    {
                        pl = (SimulationWaypointSegment)child;
                        if (pl.ParentWaypoint.VehicleID == _currentVehicleID)
                            oList.Add(pl);
                    }
                }
                foreach (Object o in oList)
                    _segmentLayer.Children.Remove((UIElement)o);

                CurrentSimulation.WaypointLists.Remove(_currentVehicleID);
                CurrentSimulation.Colors.Remove(_currentVehicleID);
                for (int i = 1; i <= CurrentSimulation.Clones[_currentVehicleID]; i++)
                    CurrentSimulation.Colors.Remove(_currentVehicleID + i);
                CurrentSimulation.Clones.Remove(_currentVehicleID);
                CurrentSimulation.CloneOffsets.Remove(_currentVehicleID);
                VehicleIDComboBox.Items.Remove(_currentVehicleID.ToString());
                if (VehicleIDComboBox.Items.Count > 0)
                    VehicleIDComboBox.SelectedIndex = 0;
                else
                    VehicleIDComboBox.SelectedIndex = -1;
                if (MapInEditMode)
                    ToggleEditMode();
                //clear waypoint
                ClearWaypoint();
            }
        }

        private void DrawVehicle(RunThreadType type, int vehicleId, double heading, double speed, double latitude, double longitude, bool alert = false)
        {
            MapPolygon mp;
            Microsoft.Maps.MapControl.WPF.Location location;
            double pointLatitude;
            double pointLongitude;
            WGS84Point wp1;
            WGS84Point wp2;
            double ninety = heading + 90.0;
            double oneEighty = heading + 180.0;
            double twoSeventy = heading + 270.0;
            if (ninety > 360.0)
                ninety -= 360.0;
            if (oneEighty > 360.0)
                oneEighty -= 360.0;
            if (twoSeventy > 360.0)
                twoSeventy -= 360.0;

            if (_vehiclePolygons.ContainsKey(vehicleId))
            {
                mp = _vehiclePolygons[vehicleId];
                _vehicleLayer.Children.Remove(mp);
            }
            else
            {
                mp = new MapPolygon();
                if (type == RunThreadType.SIMULATION)
                    _vehicleBrushes[vehicleId] = new SolidColorBrush(CurrentSimulation.Colors[vehicleId]);
                else if (type == RunThreadType.KML)
                    _vehicleBrushes[vehicleId] = new SolidColorBrush(Colors.Pink);
                mp.Locations = new LocationCollection();
                _vehiclePolygons[vehicleId] = mp;
            }
            if (alert)
            {
                mp.Fill = _alertBrush;
                mp.Stroke = _alertBrush;
            }
            else
            {
                mp.Fill = _vehicleBrushes[vehicleId];
                mp.Stroke = _vehicleBrushes[vehicleId];
            }
            mp.Locations.Clear();

            wp1 = new WGS84Point(latitude, longitude);
            wp2 = GeoVector.DestinationPoint(wp1, heading, 2.4);

            wp1 = GeoVector.DestinationPoint(wp2, ninety, 0.5);
            location = new Microsoft.Maps.MapControl.WPF.Location(wp1.Latitude, wp1.Longitude);
            mp.Locations.Add(location);

            wp2 = GeoVector.DestinationPoint(wp1, ninety, 0.4);

            wp1 = GeoVector.DestinationPoint(wp2, oneEighty, 0.5);
            location = new Microsoft.Maps.MapControl.WPF.Location(wp1.Latitude, wp1.Longitude);
            mp.Locations.Add(location);
            
            wp2 = GeoVector.DestinationPoint(wp1, oneEighty, 4.3);
            location = new Microsoft.Maps.MapControl.WPF.Location(wp2.Latitude, wp2.Longitude);
            mp.Locations.Add(location);

            wp1 = GeoVector.DestinationPoint(wp2, twoSeventy, 1.8);
            location = new Microsoft.Maps.MapControl.WPF.Location(wp1.Latitude, wp1.Longitude);
            mp.Locations.Add(location);

            wp2 = GeoVector.DestinationPoint(wp1, heading, 4.3);
            location = new Microsoft.Maps.MapControl.WPF.Location(wp2.Latitude, wp2.Longitude);
            mp.Locations.Add(location);

            wp1 = GeoVector.DestinationPoint(wp2, heading, 0.5);

            wp2 = GeoVector.DestinationPoint(wp1, ninety, 0.4);
            location = new Microsoft.Maps.MapControl.WPF.Location(wp2.Latitude, wp2.Longitude);
            mp.Locations.Add(location);

            _vehicleLayer.Children.Add(mp);
        }

        private void PushPinsLayerCheckbox_Checked(object sender, RoutedEventArgs e)
        {
            if (myMap != null)
                myMap.Children.Add(_pinLayer);
        }

        private void PathsLayerCheckbox_Checked(object sender, RoutedEventArgs e)
        {
            if (myMap != null)
                myMap.Children.Add(_segmentLayer);
        }

        private void VehiclesLayerCheckbox_Checked(object sender, RoutedEventArgs e)
        {
            if (myMap != null)
                myMap.Children.Add(_vehicleLayer);
        }

        private void KMLPushPinsLayerCheckbox_Checked(object sender, RoutedEventArgs e)
        {
            if (myMap != null)
                myMap.Children.Add(_kmlPinLayer);
        }

        private void KMLPathsLayerCheckbox_Checked(object sender, RoutedEventArgs e)
        {
            if (myMap != null)
                myMap.Children.Add(_kmlPathLayer);
        }


        private void PushPinsLayerCheckbox_Unchecked(object sender, RoutedEventArgs e)
        {
            if (myMap != null)
                myMap.Children.Remove(_pinLayer);
        }

        private void PathsLayerCheckbox_Unchecked(object sender, RoutedEventArgs e)
        {
            if (myMap != null)
                myMap.Children.Remove(_segmentLayer);
        }

        private void VehiclesLayerCheckbox_Unchecked(object sender, RoutedEventArgs e)
        {
            if (myMap != null)
                myMap.Children.Remove(_vehicleLayer);
        }

        private void KMLPushPinsLayerCheckbox_Unchecked(object sender, RoutedEventArgs e)
        {
            if (myMap != null)
                myMap.Children.Remove(_kmlPinLayer);
        }

        private void KMLPathsLayerCheckbox_Unchecked(object sender, RoutedEventArgs e)
        {
            if (myMap != null)
                myMap.Children.Remove(_kmlPathLayer);
        }

        private void KMLStartTimeSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            var slider = sender as Slider;
            double value = slider.Value;
            long startTick = _kmlMinTick + (long)(value / 100.0 * (_kmlMaxTick - _kmlMinTick));
            if (startTick != 0)
                KMLStartTimeSliderText.Text = new DateTime(startTick).ToString();
            if (KMLPathAutoRefreshCheckbox.IsChecked == true)
                KMLRefreshPath();
        }

        private void KMLEndTimeSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            var slider = sender as Slider;
            double value = slider.Value;
            long endTick = _kmlMinTick + (long)(value / 100.0 * (_kmlMaxTick - _kmlMinTick));
            if (endTick != 0)
                KMLEndTimeSliderText.Text = new DateTime(endTick).ToString();
            if (KMLPathAutoRefreshCheckbox.IsChecked == true)
                KMLRefreshPath();
        }

        private void KMLFindVehicleButton_Click(object sender, RoutedEventArgs e)
        {
            if (_kmlLastWaypointVehicleID != 0)
            {
                Microsoft.Maps.MapControl.WPF.Location loc = new Microsoft.Maps.MapControl.WPF.Location(_kmlWaypoints[_kmlLastWaypointVehicleID][_kmlLastWaypointIndex].Latitude, _kmlWaypoints[_kmlLastWaypointVehicleID][_kmlLastWaypointIndex].Longitude);
                myMap.Center = loc;
                myMap.ZoomLevel = 19;
            }
        }

        private void KMLRefreshPathButton_Click(object sender, RoutedEventArgs e)
        {
            KMLRefreshPath();
        }

    }
}
