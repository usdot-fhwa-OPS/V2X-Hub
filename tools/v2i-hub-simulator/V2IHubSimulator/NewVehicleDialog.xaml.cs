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
using System.Windows.Shapes;

namespace V2IHubSimulator
{
    /// <summary>
    /// Interaction logic for NewVehicleDialog.xaml
    /// </summary>
    public partial class NewVehicleDialog : Window
    {
        public NewVehicleDialog()
        {
            InitializeComponent();
        }
        
        public string VehicleID
        {
            get { return txtVehicleID.Text; }
            set { txtVehicleID.Text = value; }
        }
        public string NumberOfClones
        {
            get { return txtNumberOfClones.Text; }
            set { txtNumberOfClones.Text = value; }
        }
        public string CloneOffset
        {
            get { return txtCloneOffset.Text; }
            set { txtCloneOffset.Text = value; }
        }
        private void btnOk_Click(object sender, System.Windows.RoutedEventArgs e)
        {
            DialogResult = true;
            Close();
        }
        private void btnCancel_Click(object sender, System.Windows.RoutedEventArgs e)
        {
            Close();
        }
    }
}
