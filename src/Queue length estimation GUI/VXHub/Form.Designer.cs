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
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.txtTimeInterval = new System.Windows.Forms.NumericUpDown();
            this.label8 = new System.Windows.Forms.Label();
            this.radioButton_All = new System.Windows.Forms.RadioButton();
            this.radioButton_specific = new System.Windows.Forms.RadioButton();
            this.txtSpeed = new System.Windows.Forms.NumericUpDown();
            this.label6 = new System.Windows.Forms.Label();
            this.txtSW2 = new System.Windows.Forms.NumericUpDown();
            this.label5 = new System.Windows.Forms.Label();
            this.txtSW1 = new System.Windows.Forms.NumericUpDown();
            this.label4 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.txtTime = new System.Windows.Forms.NumericUpDown();
            this.label2 = new System.Windows.Forms.Label();
            this.cmdCalculate = new System.Windows.Forms.Button();
            this.cmdBrowse_output = new System.Windows.Forms.Button();
            this.txtFolder_out = new System.Windows.Forms.TextBox();
            this.label7 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.groupBox1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.txtTimeInterval)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.txtSpeed)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.txtSW2)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.txtSW1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.txtTime)).BeginInit();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 132);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(113, 25);
            this.label1.TabIndex = 1;
            this.label1.Text = "Input Folder:";
            // 
            // txtFolder_in
            // 
            this.txtFolder_in.Enabled = false;
            this.txtFolder_in.Location = new System.Drawing.Point(147, 129);
            this.txtFolder_in.Name = "txtFolder_in";
            this.txtFolder_in.Size = new System.Drawing.Size(248, 31);
            this.txtFolder_in.TabIndex = 2;
            this.txtFolder_in.Text = "C:\\Users\\ghiasia\\Documents\\Projects\\18 - 327 OpsIV V2X Hub\\Metrics\\Code\\Inputs fo" +
    "r queue length estimation";
            this.txtFolder_in.TextChanged += new System.EventHandler(this.txtFolder_TextChanged);
            // 
            // pictureBox1
            // 
            this.pictureBox1.Image = global::VXHub.Properties.Resources.Logo;
            this.pictureBox1.Location = new System.Drawing.Point(126, 12);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(210, 96);
            this.pictureBox1.TabIndex = 3;
            this.pictureBox1.TabStop = false;
            // 
            // cmdBrowse_input
            // 
            this.cmdBrowse_input.Location = new System.Drawing.Point(401, 129);
            this.cmdBrowse_input.Name = "cmdBrowse_input";
            this.cmdBrowse_input.Size = new System.Drawing.Size(37, 23);
            this.cmdBrowse_input.TabIndex = 4;
            this.cmdBrowse_input.Text = "...";
            this.cmdBrowse_input.UseVisualStyleBackColor = true;
            this.cmdBrowse_input.Click += new System.EventHandler(this.cmdBrowse_input_Click);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.txtTimeInterval);
            this.groupBox1.Controls.Add(this.label8);
            this.groupBox1.Controls.Add(this.radioButton_All);
            this.groupBox1.Controls.Add(this.radioButton_specific);
            this.groupBox1.Controls.Add(this.txtSpeed);
            this.groupBox1.Controls.Add(this.label6);
            this.groupBox1.Controls.Add(this.txtSW2);
            this.groupBox1.Controls.Add(this.label5);
            this.groupBox1.Controls.Add(this.txtSW1);
            this.groupBox1.Controls.Add(this.label4);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.txtTime);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Location = new System.Drawing.Point(12, 210);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(426, 331);
            this.groupBox1.TabIndex = 5;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Parameters   ";
            // 
            // txtTimeInterval
            // 
            this.txtTimeInterval.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.txtTimeInterval.Location = new System.Drawing.Point(237, 138);
            this.txtTimeInterval.Maximum = new decimal(new int[] {
            10000,
            0,
            0,
            0});
            this.txtTimeInterval.Name = "txtTimeInterval";
            this.txtTimeInterval.Size = new System.Drawing.Size(183, 31);
            this.txtTimeInterval.TabIndex = 12;
            this.txtTimeInterval.Value = new decimal(new int[] {
            50,
            0,
            0,
            0});
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(68, 138);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(140, 25);
            this.label8.TabIndex = 11;
            this.label8.Text = "Time Interval (s):";
            this.label8.Click += new System.EventHandler(this.Label8_Click);
            // 
            // radioButton_All
            // 
            this.radioButton_All.AutoSize = true;
            this.radioButton_All.Location = new System.Drawing.Point(11, 97);
            this.radioButton_All.Name = "radioButton_All";
            this.radioButton_All.Size = new System.Drawing.Size(108, 29);
            this.radioButton_All.TabIndex = 10;
            this.radioButton_All.TabStop = true;
            this.radioButton_All.Text = "All Times";
            this.radioButton_All.UseVisualStyleBackColor = true;
            this.radioButton_All.CheckedChanged += new System.EventHandler(this.RadioButton_All_CheckedChanged);
            // 
            // radioButton_specific
            // 
            this.radioButton_specific.AutoSize = true;
            this.radioButton_specific.Location = new System.Drawing.Point(11, 32);
            this.radioButton_specific.Name = "radioButton_specific";
            this.radioButton_specific.Size = new System.Drawing.Size(140, 29);
            this.radioButton_specific.TabIndex = 9;
            this.radioButton_specific.TabStop = true;
            this.radioButton_specific.Text = "Specific Time";
            this.radioButton_specific.UseVisualStyleBackColor = true;
            this.radioButton_specific.CheckedChanged += new System.EventHandler(this.RadioButton_specific_CheckedChanged);
            // 
            // txtSpeed
            // 
            this.txtSpeed.DecimalPlaces = 2;
            this.txtSpeed.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.txtSpeed.Location = new System.Drawing.Point(237, 285);
            this.txtSpeed.Name = "txtSpeed";
            this.txtSpeed.Size = new System.Drawing.Size(183, 31);
            this.txtSpeed.TabIndex = 8;
            this.txtSpeed.Value = new decimal(new int[] {
            32,
            0,
            0,
            65536});
            this.txtSpeed.ValueChanged += new System.EventHandler(this.TxtSpeed_ValueChanged);
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(15, 287);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(150, 25);
            this.label6.TabIndex = 7;
            this.label6.Text = "Stop Speed (ft/s):";
            this.label6.Click += new System.EventHandler(this.Label6_Click);
            // 
            // txtSW2
            // 
            this.txtSW2.DecimalPlaces = 2;
            this.txtSW2.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.txtSW2.Location = new System.Drawing.Point(237, 256);
            this.txtSW2.Maximum = new decimal(new int[] {
            0,
            0,
            0,
            0});
            this.txtSW2.Minimum = new decimal(new int[] {
            100,
            0,
            0,
            -2147483648});
            this.txtSW2.Name = "txtSW2";
            this.txtSW2.Size = new System.Drawing.Size(183, 31);
            this.txtSW2.TabIndex = 6;
            this.txtSW2.Value = new decimal(new int[] {
            35,
            0,
            0,
            -2147483648});
            this.txtSW2.ValueChanged += new System.EventHandler(this.TxtSW2_ValueChanged);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(15, 258);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(216, 25);
            this.label5.TabIndex = 5;
            this.label5.Text = "Shockwave 2 Speed (ft/s):";
            this.label5.Click += new System.EventHandler(this.Label5_Click);
            // 
            // txtSW1
            // 
            this.txtSW1.DecimalPlaces = 2;
            this.txtSW1.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.txtSW1.Location = new System.Drawing.Point(237, 227);
            this.txtSW1.Maximum = new decimal(new int[] {
            0,
            0,
            0,
            0});
            this.txtSW1.Minimum = new decimal(new int[] {
            100,
            0,
            0,
            -2147483648});
            this.txtSW1.Name = "txtSW1";
            this.txtSW1.Size = new System.Drawing.Size(183, 31);
            this.txtSW1.TabIndex = 4;
            this.txtSW1.Value = new decimal(new int[] {
            4,
            0,
            0,
            -2147483648});
            this.txtSW1.ValueChanged += new System.EventHandler(this.TxtSW1_ValueChanged);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(15, 229);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(216, 25);
            this.label4.TabIndex = 3;
            this.label4.Text = "Shockwave 1 Speed (ft/s):";
            this.label4.Click += new System.EventHandler(this.Label4_Click);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label3.Location = new System.Drawing.Point(6, 201);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(181, 25);
            this.label3.TabIndex = 2;
            this.label3.Text = "Default Parameters:";
            this.label3.Click += new System.EventHandler(this.Label3_Click);
            // 
            // txtTime
            // 
            this.txtTime.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.txtTime.Location = new System.Drawing.Point(237, 63);
            this.txtTime.Maximum = new decimal(new int[] {
            10000,
            0,
            0,
            0});
            this.txtTime.Name = "txtTime";
            this.txtTime.Size = new System.Drawing.Size(183, 31);
            this.txtTime.TabIndex = 1;
            this.txtTime.Value = new decimal(new int[] {
            880,
            0,
            0,
            0});
            this.txtTime.ValueChanged += new System.EventHandler(this.TxtTime_ValueChanged);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(107, 65);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(77, 25);
            this.label2.TabIndex = 0;
            this.label2.Text = "Time (s):";
            // 
            // cmdCalculate
            // 
            this.cmdCalculate.Location = new System.Drawing.Point(311, 547);
            this.cmdCalculate.Name = "cmdCalculate";
            this.cmdCalculate.Size = new System.Drawing.Size(121, 33);
            this.cmdCalculate.TabIndex = 6;
            this.cmdCalculate.Text = "Calculate";
            this.cmdCalculate.UseVisualStyleBackColor = true;
            this.cmdCalculate.Click += new System.EventHandler(this.cmdCalculate_Click);
            // 
            // cmdBrowse_output
            // 
            this.cmdBrowse_output.Location = new System.Drawing.Point(402, 166);
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
            this.txtFolder_out.Location = new System.Drawing.Point(147, 166);
            this.txtFolder_out.Name = "txtFolder_out";
            this.txtFolder_out.Size = new System.Drawing.Size(249, 31);
            this.txtFolder_out.TabIndex = 8;
            this.txtFolder_out.Text = "C:\\Users\\ghiasia\\Documents\\Projects\\18 - 327 OpsIV V2X Hub\\Metrics\\Code\\Outputs o" +
    "f queue length estimation";
            this.txtFolder_out.TextChanged += new System.EventHandler(this.TextBox1_TextChanged);
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(13, 169);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(128, 25);
            this.label7.TabIndex = 7;
            this.label7.Text = "Output Folder:";
            this.label7.Click += new System.EventHandler(this.Label7_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(10F, 25F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.White;
            this.ClientSize = new System.Drawing.Size(453, 592);
            this.Controls.Add(this.cmdBrowse_output);
            this.Controls.Add(this.txtFolder_out);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.cmdCalculate);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.cmdBrowse_input);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.txtFolder_in);
            this.Controls.Add(this.label1);
            this.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Name = "Form1";
            this.Text = "V2X Hub Performance Measures";
            this.Load += new System.EventHandler(this.Form1_Load);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.txtTimeInterval)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.txtSpeed)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.txtSW2)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.txtSW1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.txtTime)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox txtFolder_in;
        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.Button cmdBrowse_input;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.NumericUpDown txtSpeed;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.NumericUpDown txtSW2;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.NumericUpDown txtSW1;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.NumericUpDown txtTime;
        private System.Windows.Forms.Button cmdCalculate;
        private System.Windows.Forms.Button cmdBrowse_output;
        private System.Windows.Forms.TextBox txtFolder_out;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.RadioButton radioButton_All;
        private System.Windows.Forms.RadioButton radioButton_specific;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.NumericUpDown txtTimeInterval;
    }
}

