namespace EventWatcher
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
            this.panel1 = new System.Windows.Forms.Panel();
            this.log = new System.Windows.Forms.TextBox();
            this.info = new System.Windows.Forms.RichTextBox();
            this.lv = new System.Windows.Forms.ListView();
            this.colAddress = new System.Windows.Forms.ColumnHeader();
            this.colId = new System.Windows.Forms.ColumnHeader();
            this.colPbxId = new System.Windows.Forms.ColumnHeader();
            this.colState = new System.Windows.Forms.ColumnHeader();
            this.colCaller = new System.Windows.Forms.ColumnHeader();
            this.colOriginator = new System.Windows.Forms.ColumnHeader();
            this.colOrginatee = new System.Windows.Forms.ColumnHeader();
            this.lvAll = new System.Windows.Forms.ListView();
            this.from = new System.Windows.Forms.ColumnHeader();
            this.To = new System.Windows.Forms.ColumnHeader();
            this.ChannelId = new System.Windows.Forms.ColumnHeader();
            this.SessionId = new System.Windows.Forms.ColumnHeader();
            this.State = new System.Windows.Forms.ColumnHeader();
            this.panel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.log);
            this.panel1.Controls.Add(this.info);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Right;
            this.panel1.Location = new System.Drawing.Point(627, 0);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(293, 546);
            this.panel1.TabIndex = 11;
            // 
            // log
            // 
            this.log.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.log.Location = new System.Drawing.Point(0, 526);
            this.log.Name = "log";
            this.log.Size = new System.Drawing.Size(293, 20);
            this.log.TabIndex = 11;
            // 
            // info
            // 
            this.info.Dock = System.Windows.Forms.DockStyle.Fill;
            this.info.Location = new System.Drawing.Point(0, 0);
            this.info.Name = "info";
            this.info.Size = new System.Drawing.Size(293, 546);
            this.info.TabIndex = 10;
            this.info.Text = "";
            // 
            // lv
            // 
            this.lv.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.colAddress,
            this.colId,
            this.colPbxId,
            this.colState,
            this.colCaller,
            this.colOriginator,
            this.colOrginatee});
            this.lv.Dock = System.Windows.Forms.DockStyle.Top;
            this.lv.Location = new System.Drawing.Point(0, 0);
            this.lv.Name = "lv";
            this.lv.Size = new System.Drawing.Size(627, 152);
            this.lv.TabIndex = 12;
            this.lv.UseCompatibleStateImageBehavior = false;
            this.lv.View = System.Windows.Forms.View.Details;
            // 
            // colAddress
            // 
            this.colAddress.Text = "Anknytning";
            this.colAddress.Width = 65;
            // 
            // colId
            // 
            this.colId.Text = "Id";
            this.colId.Width = 26;
            // 
            // colPbxId
            // 
            this.colPbxId.Text = "PbxId";
            this.colPbxId.Width = 230;
            // 
            // colState
            // 
            this.colState.Text = "Status";
            this.colState.Width = 50;
            // 
            // colCaller
            // 
            this.colCaller.Text = "Caller";
            this.colCaller.Width = 62;
            // 
            // colOriginator
            // 
            this.colOriginator.Text = "Originator";
            this.colOriginator.Width = 72;
            // 
            // colOrginatee
            // 
            this.colOrginatee.Text = "Orginatee";
            this.colOrginatee.Width = 116;
            // 
            // lvAll
            // 
            this.lvAll.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.from,
            this.To,
            this.ChannelId,
            this.SessionId,
            this.State});
            this.lvAll.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lvAll.Location = new System.Drawing.Point(0, 152);
            this.lvAll.Name = "lvAll";
            this.lvAll.Size = new System.Drawing.Size(627, 394);
            this.lvAll.TabIndex = 13;
            this.lvAll.UseCompatibleStateImageBehavior = false;
            this.lvAll.View = System.Windows.Forms.View.Details;
            this.lvAll.SelectedIndexChanged += new System.EventHandler(this.lvAll_SelectedIndexChanged);
            this.lvAll.ItemSelectionChanged += new System.Windows.Forms.ListViewItemSelectionChangedEventHandler(this.lvAll_ItemSelectionChanged);
            // 
            // from
            // 
            this.from.Text = "Från";
            // 
            // To
            // 
            this.To.Text = "Till";
            this.To.Width = 84;
            // 
            // ChannelId
            // 
            this.ChannelId.Text = "ChannelId";
            this.ChannelId.Width = 282;
            // 
            // SessionId
            // 
            this.SessionId.Text = "SessionId";
            this.SessionId.Width = 318;
            // 
            // State
            // 
            this.State.Text = "State";
            this.State.Width = 99;
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(920, 546);
            this.Controls.Add(this.lvAll);
            this.Controls.Add(this.lv);
            this.Controls.Add(this.panel1);
            this.Name = "Form1";
            this.Text = "Form1";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.TextBox log;
        private System.Windows.Forms.RichTextBox info;
        private System.Windows.Forms.ListView lv;
        private System.Windows.Forms.ColumnHeader colAddress;
        private System.Windows.Forms.ColumnHeader colId;
        private System.Windows.Forms.ColumnHeader colPbxId;
        private System.Windows.Forms.ColumnHeader colState;
        private System.Windows.Forms.ColumnHeader colCaller;
        private System.Windows.Forms.ColumnHeader colOriginator;
        private System.Windows.Forms.ColumnHeader colOrginatee;
        private System.Windows.Forms.ListView lvAll;
        private System.Windows.Forms.ColumnHeader from;
        private System.Windows.Forms.ColumnHeader To;
        private System.Windows.Forms.ColumnHeader ChannelId;
        private System.Windows.Forms.ColumnHeader SessionId;
        private System.Windows.Forms.ColumnHeader State;

    }
}

