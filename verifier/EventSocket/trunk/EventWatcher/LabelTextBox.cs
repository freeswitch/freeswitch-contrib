using System;
using System.Collections.Generic;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace EventWatcher
{
    class LabelTextBox : TextBox
    {
        private string _label;

        public string Label
        {
            get { return _label; }
            set 
            { 
                _label = value; 
                if (Text == string.Empty)
                    SetLabel();
            }
        }

        public void test()
        {
            TextBox x = new TextBox();
            x.KeyDown += tb_KeyDown;
            x.TextChanged += tb_TextChanged;
        }

        private void tb_KeyDown(object sender, System.Windows.Forms.KeyEventArgs e)
        {
            if (ForeColor == Color.FromKnownColor(KnownColor.InactiveCaptionText))
                RemoveLabel();
        }

        private void tb_TextChanged(object sender, EventArgs e)
        {
            if (Text == string.Empty)
              SetLabel();
            else if (Text != _label && Text != string.Empty)
             RemoveLabel();   
        }

        private void RemoveLabel()
        {
            ForeColor = Color.FromKnownColor(KnownColor.WindowText);
            Text = string.Empty;

        }
        private void SetLabel()
        {
            ForeColor = Color.FromKnownColor(KnownColor.InactiveCaptionText);
            Text = _label;

        }

    }
}
