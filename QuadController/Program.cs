using System;
using System.Data;
using System.Threading;

namespace QuadController
{
    internal class Program
    {
        private JcRobotNetworking con;
        private Joystick js;
        private bool isRunning;
        private byte[] messageOn = { 1,0,0,0};
        private byte[] messageOff = { 0,0,0,0};
        private JcRobotNetworking.Command cmo,cmf;
        private float power = 0;
        
        public static void Main(string[] args)
        {
            Program p = new Program();
            p.Start();
        }

        public Program()
        {
            con = new JcRobotNetworking(JcRobotNetworking.ConnectionType.Controller);
            cmo = new JcRobotNetworking.Command(0, messageOn);
            cmf = new JcRobotNetworking.Command(0, messageOff);
        }

        public void Start()
        {
            isRunning = true;
            con.Connect(1296,"192.168.4.1");

            new Thread(WaitForQuit).Start();

            while (isRunning)
            {
                Loop();
            }
        }

        private void WaitForQuit()
        {
            string input = Console.ReadLine();
            while (!input.ToLower().Equals("q")&&!input.ToLower().Equals("quit"))
            {
                int inpow;
                if (input.Equals("w"))
                {
                    power += .05f;
                }else if (input.Equals("s"))
                {
                    power -= .05f;
                }
                else if (input.Equals("/"))
                {
                    power = 0;
                }else if (int.TryParse(input,out inpow))
                {
                    power = inpow/100.0f;
                }
                Console.WriteLine("power: "+power);
                input = Console.ReadLine();
            }

            isRunning = false;
        }
        
        public void Loop()
        {
            con.SendCommand(new JcRobotNetworking.Command(1,BitConverter.GetBytes(power)));
            
            Thread.Sleep(50);
        }
        
    }
}