namespace VXHub
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.label1 = new System.Windows.Forms.Label();
            this.txtFolder_in = new System.Windows.Forms.TextBox();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.cmdBrowse_input = new System.Windows.Forms.Button();
            this.cmdCalculate = new System.Windows.Forms.Button();
            this.cmdBrowse_output = new System.Windows.Forms.Button();
            this.txtFolder_out = new System.Windows.Forms.TextBox();
            this.label7 = new System.Windows.Forms.Label();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.txtJamSpacing = new System.Windows.Forms.NumericUpDown();
            this.label3 = new System.Windows.Forms.Label();
            this.txtSpeed = new System.Windows.Forms.NumericUpDown();
            this.label10 = new System.Windows.Forms.Label();
            this.txtSW2 = new System.Windows.Forms.NumericUpDown();
            this.label11 = new System.Windows.Forms.Label();
            this.txtSW1 = new System.Windows.Forms.NumericUpDown();
            this.label12 = new System.Windows.Forms.Label();
            this.txtQueueInterval = new System.Windows.Forms.NumericUpDown();
            this.label8 = new System.Windows.Forms.Label();
            this.radioButton_All = new System.Windows.Forms.RadioButton();
            this.radioButton_specific = new System.Windows.Forms.RadioButton();
            this.txtQueueTime = new System.Windows.Forms.NumericUpDown();
            this.label2 = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.checkBox_Delay = new System.Windows.Forms.CheckBox();
            this.txtDelayTo = new System.Windows.Forms.NumericUpDown();
            this.label_ToTime = new System.Windows.Forms.Label();
            this.label_FromTime = new System.Windows.Forms.Label();
            this.txtDelayFrom = new System.Windows.Forms.NumericUpDown();
            this.checkBox_Queue = new System.Windows.Forms.CheckBox();
            this.checkBox_postprocess = new System.Windows.Forms.CheckBox();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.groupBox2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.txtJamSpacing)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.txtSpeed)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.txtSW2)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.txtSW1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.txtQueueInterval)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.txtQueueTime)).BeginInit();
            this.groupBox1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.txtDelayTo)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.txtDelayFrom)).BeginInit();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 132);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(92, 20);
            this.label1.TabIndex = 1;
            this.label1.Text = "Input Folder:";
            // 
            // txtFolder_in
            // 
            this.txtFolder_in.Enabled = false;
            this.txtFolder_in.Location = new System.Drawing.Point(129, 129);
            this.txtFolder_in.Name = "txtFolder_in";
            this.txtFolder_in.Size = new System.Drawing.Size(233, 27);
            this.txtFolder_in.TabIndex = 2;
            this.txtFolder_in.TextChanged += new System.EventHandler(this.txtFolder_TextChanged);
            // 
            // pictureBox1
            // 
            this.pictureBox1.Image = global::VXHub.Properties.Resources.Logo;
            this.pictureBox1.Location = new System.Drawing.Point(119, 12);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(210, 96);
            this.pictureBox1.TabIndex = 3;
            this.pictureBox1.TabStop = false;
            // 
            // cmdBrowse_input
            // 
            this.cmdBrowse_input.Location = new System.Drawing.Point(371, 128);
            this.cmdBrowse_input.Name = "cmdBrowse_input";
            this.cmdBrowse_input.Size = new System.Drawing.Size(37, 23);
            this.cmdBrowse_input.TabIndex = 4;
            this.cmdBrowse_input.Text = "...";
            this.cmdBrowse_input.UseVisualStyleBackColor = true;
            this.cmdBrowse_input.Click += new System.EventHandler(this.cmdBrowse_input_Click);
            // 
            // cmdCalculate
            // 
            this.cmdCalculate.Location = new System.Drawing.Point(154, 658);
            this.cmdCalculate.Name = "cmdCalculate";
            this.cmdCalculate.Size = new System.Drawing.Size(121, 33);
            this.cmdCalculate.TabIndex = 6;
            this.cmdCalculate.Text = "Calculate";
            this.cmdCalculate.UseVisualStyleBackColor = true;
            this.cmdCalculate.Click += new System.EventHandler(this.cmdCalculate_Click);
            // 
            // cmdBrowse_output
            // 
            this.cmdBrowse_output.Location = new System.Drawing.Point(371, 165);
            this.cmdBrowse_output.Name = "cmdBrowse_output";
            this.cmdBrowse_output.Size = new System.Drawing.Size(37, 23);
            this.cmdBrowse_output.TabIndex = 9;
            this.cmdBrowse_output.Text = "...";
            this.cmdBrowse_output.UseVisualStyleBackColor = true;
            this.cmdBrowse_output.Click += new System.EventHandler(this.CmdBrowse_output_Click);
            // 
            // txtFolder_out
            // 
            this.txtFolder_out.Enabled = false;
            this.txtFolder_out.Location = new System.Drawing.Point(129, 166);
            this.txtFolder_out.Name = "txtFolder_out";
            this.txtFolder_out.Size = new System.Drawing.Size(233, 27);
            this.txtFolder_out.TabIndex = 8;
            this.txtFolder_out.TextChanged += new System.EventHandler(this.TextBox1_TextChanged);
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(13, 169);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(104, 20);
            this.label7.TabIndex = 7;
            this.label7.Text = "Output Folder:";
            this.label7.Click += new System.EventHandler(this.Label7_Click);
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.txtJamSpacing);
            this.groupBox2.Controls.Add(this.label3);
            this.groupBox2.Controls.Add(this.txtSpeed);
            this.groupBox2.Controls.Add(this.label10);
            this.groupBox2.Controls.Add(this.txtSW2);
            this.groupBox2.Controls.Add(this.label11);
            this.groupBox2.Controls.Add(this.txtSW1);
            this.groupBox2.Controls.Add(this.label12);
            this.groupBox2.Location = new System.Drawing.Point(12, 504);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(396, 148);
            this.groupBox2.TabIndex = 13;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Parameters   ";
            this.groupBox2.Enter += new System.EventHandler(this.groupBox2_Enter);
            // 
            // txtJamSpacing
            // 
            this.txtJamSpacing.DecimalPlaces = 2;
            this.txtJamSpacing.Enabled = false;
            this.txtJamSpacing.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.txtJamSpacing.Location = new System.Drawing.Point(203, 109);
            this.txtJamSpacing.Name = "txtJamSpacing";
            this.txtJamSpacing.Size = new System.Drawing.Size(183, 27);
            this.txtJamSpacing.TabIndex = 10;
            this.txtJamSpacing.Value = new decimal(new int[] {
            25,
            0,
            0,
            0});
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(6, 111);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(116, 20);
            this.label3.TabIndex = 9;
            this.label3.Text = "Jam Spacing (ft)";
            // 
            // txtSpeed
            // 
            this.txtSpeed.DecimalPlaces = 2;
            this.txtSpeed.Enabled = false;
            this.txtSpeed.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.txtSpeed.Location = new System.Drawing.Point(203, 80);
            this.txtSpeed.Name = "txtSpeed";
            this.txtSpeed.Size = new System.Drawing.Size(183, 27);
            this.txtSpeed.TabIndex = 8;
            this.txtSpeed.Value = new decimal(new int[] {
            32,
            0,
            0,
            65536});
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(6, 82);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(125, 20);
            this.label10.TabIndex = 7;
            this.label10.Text = "Stop Speed (ft/s):";
            // 
            // txtSW2
            // 
            this.txtSW2.DecimalPlaces = 2;
            this.txtSW2.Enabled = false;
            this.txtSW2.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.txtSW2.Location = new System.Drawing.Point(203, 51);
            this.txtSW2.Name = "txtSW2";
            this.txtSW2.Size = new System.Drawing.Size(183, 27);
            this.txtSW2.TabIndex = 6;
            this.txtSW2.Value = new decimal(new int[] {
            35,
            0,
            0,
            0});
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Location = new System.Drawing.Point(6, 53);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(179, 20);
            this.label11.TabIndex = 5;
            this.label11.Text = "Shockwave 2 Speed (ft/s):";
            // 
            // txtSW1
            // 
            this.txtSW1.DecimalPlaces = 2;
            this.txtSW1.Enabled = false;
            this.txtSW1.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.txtSW1.Location = new System.Drawing.Point(203, 22);
            this.txtSW1.Name = "txtSW1";
            this.txtSW1.Size = new System.Drawing.Size(183, 27);
            this.txtSW1.TabIndex = 4;
            this.txtSW1.Value = new decimal(new int[] {
            4,
            0,
            0,
            0});
            // 
            // label12
            // 
            this.label12.AutoSize = true;
            this.label12.Location = new System.Drawing.Point(6, 24);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(179, 20);
            this.label12.TabIndex = 3;
            this.label12.Text = "Shockwave 1 Speed (ft/s):";
            // 
            // txtQueueInterval
            // 
            this.txtQueueInterval.Enabled = false;
            this.txtQueueInterval.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.txtQueueInterval.Location = new System.Drawing.Point(203, 123);
            this.txtQueueInterval.Maximum = new decimal(new int[] {
            10000,
            0,
            0,
            0});
            this.txtQueueInterval.Name = "txtQueueInterval";
            this.txtQueueInterval.Size = new System.Drawing.Size(183, 27);
            this.txtQueueInterval.TabIndex = 19;
            this.txtQueueInterval.Value = new decimal(new int[] {
            60,
            0,
            0,
            0});
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(95, 125);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(118, 20);
            this.label8.TabIndex = 18;
            this.label8.Text = "Time Interval (s):";
            // 
            // radioButton_All
            // 
            this.radioButton_All.AutoSize = true;
            this.radioButton_All.Enabled = false;
            this.radioButton_All.Location = new System.Drawing.Point(41, 98);
            this.radioButton_All.Name = "radioButton_All";
            this.radioButton_All.Size = new System.Drawing.Size(91, 24);
            this.radioButton_All.TabIndex = 17;
            this.radioButton_All.TabStop = true;
            this.radioButton_All.Text = "All Times";
            this.radioButton_All.UseVisualStyleBackColor = true;
            // 
            // radioButton_specific
            // 
            this.radioButton_specific.AutoSize = true;
            this.radioButton_specific.Enabled = false;
            this.radioButton_specific.Location = new System.Drawing.Point(41, 51);
            this.radioButton_specific.Name = "radioButton_specific";
            this.radioButton_specific.Size = new System.Drawing.Size(119, 24);
            this.radioButton_specific.TabIndex = 16;
            this.radioButton_specific.TabStop = true;
            this.radioButton_specific.Text = "Specific Time";
            this.radioButton_specific.UseVisualStyleBackColor = true;
            // 
            // txtQueueTime
            // 
            this.txtQueueTime.Enabled = false;
            this.txtQueueTime.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.txtQueueTime.Location = new System.Drawing.Point(203, 73);
            this.txtQueueTime.Maximum = new decimal(new int[] {
            1215752192,
            23,
            0,
            0});
            this.txtQueueTime.Name = "txtQueueTime";
            this.txtQueueTime.Size = new System.Drawing.Size(183, 27);
            this.txtQueueTime.TabIndex = 15;
            this.txtQueueTime.Value = new decimal(new int[] {
            1573145075,
            0,
            0,
            0});
            this.txtQueueTime.ValueChanged += new System.EventHandler(this.txtQueueTime_ValueChanged);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(137, 75);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(65, 20);
            this.label2.TabIndex = 14;
            this.label2.Text = "Time (s):";
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.checkBox_Delay);
            this.groupBox1.Controls.Add(this.txtDelayTo);
            this.groupBox1.Controls.Add(this.label_ToTime);
            this.groupBox1.Controls.Add(this.label_FromTime);
            this.groupBox1.Controls.Add(this.txtDelayFrom);
            this.groupBox1.Controls.Add(this.checkBox_Queue);
            this.groupBox1.Controls.Add(this.txtQueueInterval);
            this.groupBox1.Controls.Add(this.label8);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.radioButton_All);
            this.groupBox1.Controls.Add(this.txtQueueTime);
            this.groupBox1.Controls.Add(this.radioButton_specific);
            this.groupBox1.Location = new System.Drawing.Point(12, 239);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(396, 259);
            this.groupBox1.TabIndex = 20;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Performance Measures";
            // 
            // checkBox_Delay
            // 
            this.checkBox_Delay.AutoSize = true;
            this.checkBox_Delay.Location = new System.Drawing.Point(16, 163);
            this.checkBox_Delay.Name = "checkBox_Delay";
            this.checkBox_Delay.Size = new System.Drawing.Size(128, 24);
            this.checkBox_Delay.TabIndex = 20;
            this.checkBox_Delay.Text = "Average Delay";
            this.checkBox_Delay.UseVisualStyleBackColor = true;
            this.checkBox_Delay.CheckedChanged += new System.EventHandler(this.checkBox_Delay_CheckedChanged);
            // 
            // txtDelayTo
            // 
            this.txtDelayTo.Enabled = false;
            this.txtDelayTo.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.txtDelayTo.Location = new System.Drawing.Point(203, 223);
            this.txtDelayTo.Maximum = new decimal(new int[] {
            1215752192,
            23,
            0,
            0});
            this.txtDelayTo.Name = "txtDelayTo";
            this.txtDelayTo.Size = new System.Drawing.Size(183, 27);
            this.txtDelayTo.TabIndex = 26;
            this.txtDelayTo.Value = new decimal(new int[] {
            1573145100,
            0,
            0,
            0});
            // 
            // label_ToTime
            // 
            this.label_ToTime.AutoSize = true;
            this.label_ToTime.Location = new System.Drawing.Point(38, 225);
            this.label_ToTime.Name = "label_ToTime";
            this.label_ToTime.Size = new System.Drawing.Size(85, 20);
            this.label_ToTime.TabIndex = 25;
            this.label_ToTime.Text = "To Time (s):";
            // 
            // label_FromTime
            // 
            this.label_FromTime.AutoSize = true;
            this.label_FromTime.Location = new System.Drawing.Point(38, 194);
            this.label_FromTime.Name = "label_FromTime";
            this.label_FromTime.Size = new System.Drawing.Size(103, 20);
            this.label_FromTime.TabIndex = 21;
            this.label_FromTime.Text = "From Time (s):";
            // 
            // txtDelayFrom
            // 
            this.txtDelayFrom.Enabled = false;
            this.txtDelayFrom.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.txtDelayFrom.Location = new System.Drawing.Point(203, 192);
            this.txtDelayFrom.Maximum = new decimal(new int[] {
            1215752192,
            23,
            0,
            0});
            this.txtDelayFrom.Name = "txtDelayFrom";
            this.txtDelayFrom.Size = new System.Drawing.Size(183, 27);
            this.txtDelayFrom.TabIndex = 22;
            this.txtDelayFrom.Value = new decimal(new int[] {
            1573145000,
            0,
            0,
            0});
            // 
            // checkBox_Queue
            // 
            this.checkBox_Queue.AutoSize = true;
            this.checkBox_Queue.Location = new System.Drawing.Point(16, 26);
            this.checkBox_Queue.Name = "checkBox_Queue";
            this.checkBox_Queue.Size = new System.Drawing.Size(123, 24);
            this.checkBox_Queue.TabIndex = 0;
            this.checkBox_Queue.Text = "Queue Length";
            this.checkBox_Queue.UseVisualStyleBackColor = true;
            this.checkBox_Queue.CheckedChanged += new System.EventHandler(this.checkBox_Queue_CheckedChanged);
            // 
            // checkBox_postprocess
            // 
            this.checkBox_postprocess.AutoSize = true;
            this.checkBox_postprocess.Location = new System.Drawing.Point(18, 203);
            this.checkBox_postprocess.Name = "checkBox_postprocess";
            this.checkBox_postprocess.Size = new System.Drawing.Size(195, 24);
            this.checkBox_postprocess.TabIndex = 21;
            this.checkBox_postprocess.Text = "Use post-processed data";
            this.checkBox_postprocess.UseVisualStyleBackColor = true;
            this.checkBox_postprocess.CheckedChanged += new System.EventHandler(this.checkBox_postprocess_CheckedChanged);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.White;
            this.ClientSize = new System.Drawing.Size(417, 701);
            this.Controls.Add(this.checkBox_postprocess);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.cmdBrowse_output);
            this.Controls.Add(this.txtFolder_out);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.cmdCalculate);
            this.Controls.Add(this.cmdBrowse_input);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.txtFolder_in);
            this.Controls.Add(this.label1);
            this.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Name = "Form1";
            this.Text = "V2X Hub Performance Measures";
            this.Load += new System.EventHandler(this.Form1_Load);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.txtJamSpacing)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.txtSpeed)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.txtSW2)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.txtSW1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.txtQueueInterval)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.txtQueueTime)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.txtDelayTo)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.txtDelayFrom)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox txtFolder_in;
        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.Button cmdBrowse_input;
        private System.Windows.Forms.Button cmdCalculate;
        private System.Windows.Forms.Button cmdBrowse_output;
        private System.Windows.Forms.TextBox txtFolder_out;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.NumericUpDown txtSpeed;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.NumericUpDown txtSW2;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.NumericUpDown txtSW1;
        private System.Windows.Forms.Label label12;
        private System.Windows.Forms.NumericUpDown txtJamSpacing;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.NumericUpDown txtQueueInterval;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.RadioButton radioButton_All;
        private System.Windows.Forms.RadioButton radioButton_specific;
        private System.Windows.Forms.NumericUpDown txtQueueTime;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.CheckBox checkBox_Queue;
        private System.Windows.Forms.CheckBox checkBox_Delay;
        private System.Windows.Forms.NumericUpDown txtDelayTo;
        private System.Windows.Forms.Label label_ToTime;
        private System.Windows.Forms.Label label_FromTime;
        private System.Windows.Forms.NumericUpDown txtDelayFrom;
        private System.Windows.Forms.CheckBox checkBox_postprocess;
    }
}

