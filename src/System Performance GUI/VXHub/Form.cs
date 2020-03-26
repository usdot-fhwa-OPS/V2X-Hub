using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace VXHub
{
    public partial class Form1 : Form
    {
        Boolean check_postprocess;
        Boolean specific_time;
        Boolean check_queue;
        Boolean check_delay;

        public Form1()
        {
            InitializeComponent();
        }

        private void cmdBrowse_input_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog dlg = new FolderBrowserDialog();
            if (dlg.ShowDialog() == DialogResult.OK)
            {
                txtFolder_in.Text = dlg.SelectedPath;
            } else
            {
                txtFolder_in.Text = "";
            }
        }
        private void CmdBrowse_output_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog dlg = new FolderBrowserDialog();
            if (dlg.ShowDialog() == DialogResult.OK)
            {
                txtFolder_out.Text = dlg.SelectedPath;
            }
            else
            {
                txtFolder_out.Text = "";
            }
        }

        private void txtFolder_TextChanged(object sender, EventArgs e)
        {
            cmdCalculate.Enabled = Directory.Exists(txtFolder_in.Text);
        }
        private void TextBox1_TextChanged(object sender, EventArgs e)
        {
            cmdCalculate.Enabled = Directory.Exists(txtFolder_out.Text);
        }
        private void checkBox_postprocess_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox_postprocess.Checked == true)
            {
                check_postprocess = true;
            }
            else
            {
                check_postprocess = false;
            }
        }
        private void checkBox_Queue_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox_Queue.Checked == true)
            {
                txtQueueTime.Enabled = true;
                txtQueueInterval.Enabled = true;
                txtSW1.Enabled = true;
                txtSW2.Enabled = true;
                txtSpeed.Enabled = true;
                check_queue = true;
                radioButton_specific.Enabled = true;
                radioButton_All.Enabled = true;
            }
            else if (checkBox_Queue.Checked == false && checkBox_Delay.Checked == true)
            {
                txtQueueTime.Enabled = false;
                txtQueueInterval.Enabled = false;
                radioButton_specific.Enabled = false;
                radioButton_All.Enabled = false;
                check_queue = false;
            }
            else if (checkBox_Queue.Checked == false && checkBox_Delay.Checked == false)
            {
                txtQueueTime.Enabled = false;
                txtQueueInterval.Enabled = false;
                radioButton_specific.Enabled = false;
                radioButton_All.Enabled = false;
                txtSW1.Enabled = false;
                txtSW2.Enabled = false;
                txtSpeed.Enabled = false;
                check_queue = false;
            }

        }
        private void checkBox_Delay_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox_Delay.Checked == true)
            {
                txtDelayFrom.Enabled = true;
                txtDelayTo.Enabled = true;
                txtSW1.Enabled = true;
                txtSW2.Enabled = true;
                txtSpeed.Enabled = true;
                txtJamSpacing.Enabled = true;
                check_delay = true;
            }
            else if (checkBox_Delay.Checked == false && checkBox_Queue.Checked == true)
            {
                txtDelayFrom.Enabled = false;
                txtDelayTo.Enabled = false;
                txtJamSpacing.Enabled = false;
                check_delay = false;
            }
            else if(checkBox_Delay.Checked == false && checkBox_Queue.Checked == false)
            {
                txtDelayFrom.Enabled = false;
                txtDelayTo.Enabled = false;
                txtSW1.Enabled = false;
                txtSW2.Enabled = false;
                txtSpeed.Enabled = false;
                txtJamSpacing.Enabled = false;
                check_delay = false;
            }
        }

        private void RadioButton_specific_CheckedChanged(object sender, EventArgs e)
        {
            specific_time = true;
        }
        private void RadioButton_All_CheckedChanged(object sender, EventArgs e)
        {
            specific_time = false;
        }

        private void cmdCalculate_Click(object sender, EventArgs e)
        {
            String scriptPath = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            scriptPath += @"\Scripts\queue_delay_estimation_v5.py";

            scriptPath = "\"" + scriptPath + "\""; 
            String path_input = "\"" + txtFolder_in.Text + "\"";
            String path_output = "\"" + txtFolder_out.Text + "\"";

            String queue_time = txtQueueTime.Value.ToString();
            String queue_interval = txtQueueInterval.Value.ToString();
            String delay_from = txtDelayFrom.Value.ToString();
            String delay_to = txtDelayTo.Value.ToString();
            String sw1 = txtSW1.Value.ToString();
            String sw2 = txtSW2.Value.ToString();
            String speed = txtSpeed.Value.ToString();
            String jam_spacing = txtJamSpacing.Value.ToString();

            String arguments = $"{scriptPath} {path_input} {path_output} {this.check_postprocess} {this.check_queue} {this.check_delay} {this.specific_time} {queue_time} {queue_interval} {delay_from} {delay_to} {sw1} {sw2} {speed} {jam_spacing}";

            try
            {
                ProcessStartInfo info = new ProcessStartInfo();
                info.Arguments = arguments;
                info.FileName = @"C:\Users\ghiasia\AppData\Local\Programs\Python\Python36\python.exe";
                info.WindowStyle = ProcessWindowStyle.Maximized;

                Process process = Process.Start(info);
                process.Exited += Process_Exited;
                process.Start();

                cmdCalculate.Text = "Please Wait";
                cmdCalculate.Enabled = false;
                cmdBrowse_input.Enabled = false;
            } catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            }
        }

        private void Process_Exited(object sender, EventArgs e)
        {
            if (InvokeRequired)
            {
                Invoke(new Action(() =>
                {
                    ReEnable();
                }));
            }
            else
            {
                ReEnable();
            }
        }

        private void ReEnable()
        {
            cmdCalculate.Text = "Calculate";
            cmdCalculate.Enabled = true;
            cmdBrowse_input.Enabled = true;
        }

        private void Label7_Click(object sender, EventArgs e)
        {

        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        private void Label6_Click(object sender, EventArgs e)
        {

        }

        private void Label5_Click(object sender, EventArgs e)
        {

        }

        private void Label4_Click(object sender, EventArgs e)
        {

        }

        private void Label3_Click(object sender, EventArgs e)
        {

        }

        private void TxtSW1_ValueChanged(object sender, EventArgs e)
        {

        }

        private void TxtSW2_ValueChanged(object sender, EventArgs e)
        {

        }

        private void TxtSpeed_ValueChanged(object sender, EventArgs e)
        {

        }

        private void Label8_Click(object sender, EventArgs e)
        {

        }

        private void TxtTime_ValueChanged(object sender, EventArgs e)
        {

        }

        private void groupBox2_Enter(object sender, EventArgs e)
        {

        }

        private void txtQueueTime_ValueChanged(object sender, EventArgs e)
        {

        }

        
    }
}
