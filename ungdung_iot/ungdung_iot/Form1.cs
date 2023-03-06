using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using uPLibrary.Networking.M2Mqtt;
using uPLibrary.Networking.M2Mqtt.Messages;
using System.Data.SqlClient;



namespace ungdung_iot
{
    public partial class Form1 : Form
    {
        public Form1()
        {
         
          InitializeComponent();
        }

        public static string connectionString = @"Data Source=DESKTOP-HPODA35;Initial Catalog=cambien;Integrated Security=True";
        private int dem1 = 1;

        static MqttClient client;

        private void Form1_Load(object sender, EventArgs e)
        {
            // TODO: This line of code loads data into the 'cambienDataSet.TBCB' table. You can move, or remove it, as needed.
            //this.tBCBTableAdapter.Fill(this.cambienDataSet.TBCB);
            HT();
            dataGridView1.Sort(dataGridView1.Columns[0], ListSortDirection.Descending);
            timer1.Start();
            try
            {
                client = new MqttClient("192.168.1.34", int.Parse("1883"), false, MqttSslProtocols.None, null, null);
                client.ProtocolVersion = MqttProtocolVersion.Version_3_1;
                byte code = client.Connect(Guid.NewGuid().ToString(), "KhaiGau", "29032001");
                if (code == 0)
                {
                    client.MqttMsgPublishReceived += client_MqttMsgPublishReceived;
                    client.Subscribe(new string[] { "demo" }, new byte[] { MqttMsgBase.QOS_LEVEL_EXACTLY_ONCE });
                    client.Subscribe(new string[] { "cambien" }, new byte[] { MqttMsgBase.QOS_LEVEL_EXACTLY_ONCE });
                    pcon.Image = ungdung_iot.Properties.Resources.link;

                }
                else
                {
                    //MessageBox.Show(this, "Connect Fail", "Message", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    pcon.Image = ungdung_iot.Properties.Resources.disconnected;
                }
            }
            catch (Exception)
            {
                //MessageBox.Show(this, "Wrong Format", "Message", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }


        Action<string, string> ReceiveAction;
        void client_MqttMsgPublishReceived(object sender, MqttMsgPublishEventArgs e)
        {
            ReceiveAction = Receive;
            try
            {
                this.BeginInvoke(ReceiveAction, Encoding.UTF8.GetString(e.Message), e.Topic);
            }
            catch { };
        }

        private void button1_Click(object sender, EventArgs e)
        {
            dem1++;
            if(dem1%2==0) client.Publish("demo", Encoding.UTF8.GetBytes("1"));
            else client.Publish("demo", Encoding.UTF8.GetBytes("0"));
        }


        void Receive(string message, string topic)   //xử lý dữ liệu nhân được  
        {

            if (topic == "cambien")
            {
                lbcambien.Text = message;
                using (SqlConnection connection = new SqlConnection(connectionString))
                {
                    string query = "INSERT INTO TBCB VALUES (@GIATRICB, @TIME, @DATETIME)";
                    connection.Open();
                    SqlCommand command = new SqlCommand(query, connection);
                    command.Parameters.AddWithValue("GIATRICB", lbcambien.Text.TrimEnd());
                    command.Parameters.AddWithValue("TIME",  DateTime.Now.ToString("hh:mm:ss tt"));
                    command.Parameters.AddWithValue("DATETIME", DateTime.Now.ToString("dd/MM/yyyy"));
                    command.ExecuteReader();
                    connection.Close();

                }
                HT();

            }//end topic cambien

        }

        public void HT()
        {
            string sql = "SELECT * FROM TBCB";
            SqlConnection connection = new SqlConnection(connectionString);
            SqlDataAdapter dataadapter = new SqlDataAdapter(sql, connection);
            DataSet ds = new DataSet();
            connection.Open();
            dataadapter.Fill(ds, " ");
            connection.Close();
            dataGridView1.DataSource = ds;
            dataGridView1.DataMember = " ";
            dataGridView1.Sort(dataGridView1.Columns[0], ListSortDirection.Descending);

            dataGridView1.DefaultCellStyle.WrapMode = DataGridViewTriState.True;
            dataGridView1.Columns[1].DefaultCellStyle.Alignment = DataGridViewContentAlignment.MiddleRight;

            dataGridView1.SelectionMode = DataGridViewSelectionMode.FullRowSelect;
            dataGridView1.AllowUserToResizeColumns = false;

        }


        private void timer1_Tick(object sender, EventArgs e)
        {
          //lbled.Text =   DateTime.Now.ToString("hh:mm:ss tt");
          //lbled.Text = DateTime.Now.ToString("dd/MM/yyyy");
        }
    }//end form



}//end chuong trinh
