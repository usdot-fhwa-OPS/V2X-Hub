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
        public Form1()
        {
            InitializeComponent();
        }

        private void cmdBrowse_Click(object sender, EventArgs e)
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

        private void txtFolder_TextChanged(object sender, EventArgs e)
        {
            cmdCalculate.Enabled = Directory.Exists(txtFolder_in.Text);
        }
        private void TextBox1_TextChanged(object sender, EventArgs e)
        {
            cmdCalculate.Enabled = Directory.Exists(txtFolder_out.Text);
        }

        private void cmdCalculate_Click(object sender, EventArgs e)
        {
            String scriptPath = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            scriptPath += @"\Scripts\queue_estimation_v3.py";

            scriptPath = "\"" + scriptPath + "\""; 
            String path_input = "\"" + txtFolder_in.Text + "\"";
            String path_output = "\"" + txtFolder_out.Text + "\"";

            String time = txtTime.Value.ToString();
            String sw1 = txtSW1.Value.ToString();
            String sw2 = txtSW2.Value.ToString();
            String speed = txtSpeed.Value.ToString();

            String arguments = $"{scriptPath} {path_input} {path_output} {time} {sw1} {sw2} {speed}";

            try
            {
                ProcessStartInfo info = new ProcessStartInfo();
                info.Arguments = arguments;
                info.FileName = @"C:\ProgramData\Anaconda3\python.exe";
                info.WindowStyle = ProcessWindowStyle.Maximized;

                Process process = Process.Start(info);
                process.Exited += Process_Exited;
                process.Start();

                cmdCalculate.Text = "Please Wait";
                cmdCalculate.Enabled = false;
                cmdBrowse.Enabled = false;
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
            cmdBrowse.Enabled = true;
        }

        private void Label7_Click(object sender, EventArgs e)
        {

        }


    }
}
