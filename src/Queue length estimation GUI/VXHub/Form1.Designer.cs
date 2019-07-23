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
            this.cmdBrowse = new System.Windows.Forms.Button();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
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
            this.button1 = new System.Windows.Forms.Button();
            this.txtFolder_out = new System.Windows.Forms.TextBox();
            this.label7 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.groupBox1.SuspendLayout();
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
            // cmdBrowse
            // 
            this.cmdBrowse.Location = new System.Drawing.Point(401, 129);
            this.cmdBrowse.Name = "cmdBrowse";
            this.cmdBrowse.Size = new System.Drawing.Size(37, 23);
            this.cmdBrowse.TabIndex = 4;
            this.cmdBrowse.Text = "...";
            this.cmdBrowse.UseVisualStyleBackColor = true;
            this.cmdBrowse.Click += new System.EventHandler(this.cmdBrowse_Click);
            // 
            // groupBox1
            // 
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
            this.groupBox1.Size = new System.Drawing.Size(426, 175);
            this.groupBox1.TabIndex = 5;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Parameters   ";
            // 
            // txtSpeed
            // 
            this.txtSpeed.DecimalPlaces = 2;
            this.txtSpeed.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.txtSpeed.Location = new System.Drawing.Point(237, 144);
            this.txtSpeed.Name = "txtSpeed";
            this.txtSpeed.Size = new System.Drawing.Size(183, 31);
            this.txtSpeed.TabIndex = 8;
            this.txtSpeed.Value = new decimal(new int[] {
            32,
            0,
            0,
            65536});
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(15, 146);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(150, 25);
            this.label6.TabIndex = 7;
            this.label6.Text = "Stop Speed (ft/s):";
            // 
            // txtSW2
            // 
            this.txtSW2.DecimalPlaces = 2;
            this.txtSW2.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.txtSW2.Location = new System.Drawing.Point(237, 115);
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
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(15, 117);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(216, 25);
            this.label5.TabIndex = 5;
            this.label5.Text = "Shockwave 2 Speed (ft/s):";
            // 
            // txtSW1
            // 
            this.txtSW1.DecimalPlaces = 2;
            this.txtSW1.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.txtSW1.Location = new System.Drawing.Point(237, 86);
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
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(15, 88);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(216, 25);
            this.label4.TabIndex = 3;
            this.label4.Text = "Shockwave 1 Speed (ft/s):";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label3.Location = new System.Drawing.Point(6, 60);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(181, 25);
            this.label3.TabIndex = 2;
            this.label3.Text = "Default Parameters:";
            // 
            // txtTime
            // 
            this.txtTime.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.txtTime.Location = new System.Drawing.Point(237, 26);
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
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(15, 28);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(77, 25);
            this.label2.TabIndex = 0;
            this.label2.Text = "Time (s):";
            // 
            // cmdCalculate
            // 
            this.cmdCalculate.Location = new System.Drawing.Point(317, 391);
            this.cmdCalculate.Name = "cmdCalculate";
            this.cmdCalculate.Size = new System.Drawing.Size(121, 33);
            this.cmdCalculate.TabIndex = 6;
            this.cmdCalculate.Text = "Calculate";
            this.cmdCalculate.UseVisualStyleBackColor = true;
            this.cmdCalculate.Click += new System.EventHandler(this.cmdCalculate_Click);
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(402, 166);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(37, 23);
            this.button1.TabIndex = 9;
            this.button1.Text = "...";
            this.button1.UseVisualStyleBackColor = true;
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
            this.ClientSize = new System.Drawing.Size(453, 436);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.txtFolder_out);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.cmdCalculate);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.cmdBrowse);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.txtFolder_in);
            this.Controls.Add(this.label1);
            this.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Name = "Form1";
            this.Text = "V2X Hub Performance Measures";
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
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
        private System.Windows.Forms.Button cmdBrowse;
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
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.TextBox txtFolder_out;
        private System.Windows.Forms.Label label7;
    }
}

