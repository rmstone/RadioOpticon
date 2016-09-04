using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO.Ports;

namespace RadioOpticonSerialData
{
    public partial class Form1 : Form
    {
        delegate void SetTextCallback( string[] values );

        SerialPort port = null;
        Label[] labels;

        public Form1()
        {
            InitializeComponent();
            refreshButton_Click( null, null );
            labels = new Label[16] { p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15 };
            foreach( Label lbl in labels )
            {
                lbl.BackColor = Color.Black;
                lbl.ForeColor = Color.White;
            }
            port = new SerialPort( commPort.Text, 115200, Parity.None, 8, StopBits.One );
            port.Handshake = Handshake.None;
            //port.DataReceived += DecodeData;
            port.ReadTimeout = 1000;
        }

        private void openButton_Click( object sender, EventArgs e )
        {
            if( port != null && port.IsOpen )
                port.Close();
            port.PortName = ( string )commPort.SelectedItem;
            try
            {
                port.Open();
                port.DtrEnable = true;
            }
            catch( Exception ex )
            {
                Console.WriteLine( ex.Message );
                //ret = false;
            }

        }

        private void refreshButton_Click( object sender, EventArgs e )
        {
            commPort.Items.Clear();
            commPort.Items.AddRange( SerialPort.GetPortNames() );
            commPort.SelectedIndex = 0;
        }

        private void UpdateLabels( string[] values )
        {
            if( labels[0].InvokeRequired )
            {
                SetTextCallback d = new SetTextCallback( UpdateLabels );
                this.Invoke( d, new object[] { values } );
            }
            else
            {
                for( int i = 0; i < labels.Length; i++ )
                {
                    labels[i].Text = int.Parse( values[i + 1] ) / 10.0 + " degC";
                    labels[i].BackColor = Color.FromArgb( ( int )( 255 * ( int.Parse( values[i + 1] ) / 400.0 ) ), 0, 0 );
                }
                distanceLabel.Text = values[17] + " cm";
                ambientLabel.Text = int.Parse( values[0] ) / 10.0 + " degC";
                angleLabel.Text = values[18].Trim() + " deg";
            }
        }

        public void DecodeData( object sender, SerialDataReceivedEventArgs e )
        {
            string data = port.ReadLine();
            string[] values = data.Split( new char[] { ',' }, StringSplitOptions.RemoveEmptyEntries );
            if( values.Length < 18 )
                return;
            UpdateLabels( values );
        }

        private void updateButton_Click( object sender, EventArgs e )
        {
            if( port != null )
            {
                port.Write( Encoding.ASCII.GetBytes("sensor\n"), 0, 7 );
                string data = port.ReadLine();
                string[] values = data.Split( new char[] { ',' }, StringSplitOptions.RemoveEmptyEntries );
                if( values.Length < 18 )
                    return;
                UpdateLabels( values );
            }
        }
    }
}
