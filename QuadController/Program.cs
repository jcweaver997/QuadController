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
        private bool quadEnabled = false;
        private float power = 0;
        private float kp, ki, kd, rollOffset,pitchOffset;
        private SettingState _settingState = SettingState.offset;
        private enum SettingState
        {
            offset, kp,ki,kd
        }
        
        public static void Main(string[] args)
        {
            Program p = new Program();
            p.Start();
        }

        public Program()
        {
            con = new JcRobotNetworking(JcRobotNetworking.ConnectionType.Controller);
        }

        public void Start()
        {
            isRunning = true;
            con.Connect(1296,"192.168.4.1");
            
            js = new LogitechController("/dev/input/js0");
            con.SendCommand(new JcRobotNetworking.Command(1,BitConverter.GetBytes((uint)0))); // power
            con.SendCommand(new JcRobotNetworking.Command(2,BitConverter.GetBytes(kp))); // kp
            con.SendCommand(new JcRobotNetworking.Command(3,BitConverter.GetBytes(kd))); // kd
            con.SendCommand(new JcRobotNetworking.Command(4,BitConverter.GetBytes(ki))); // ki
            con.SendCommand(new JcRobotNetworking.Command(5,BitConverter.GetBytes(rollOffset))); // roll
            con.SendCommand(new JcRobotNetworking.Command(6,BitConverter.GetBytes(pitchOffset))); // pitch
          
            new Thread(WaitForQuit).Start();

            while (isRunning)
            {
                Loop();
            }
        }

        private void WaitForQuit()
        {
            while (true)
            {
                if (js.GetButtonStart())
            {
                if (quadEnabled)
                {
                    Console.WriteLine("Disabled");
                    quadEnabled = false;
                    con.SendCommand(new JcRobotNetworking.Command(0,BitConverter.GetBytes((uint)0)));
                }
                else
                {
                    Console.WriteLine("Enabled");
                    quadEnabled = true;
                    con.SendCommand(new JcRobotNetworking.Command(0,BitConverter.GetBytes((uint)1296)));
                }
            }

            if (js.GetButtonColorLeft())
            {
                Console.WriteLine("Mode switched to offset");
                _settingState = SettingState.offset;
            }
            
            if (js.GetButtonColorBottom())
            {
                Console.WriteLine("Mode switched to kp");
                _settingState = SettingState.kp;
            }
            
            if (js.GetButtonColorRight())
            {
                Console.WriteLine("Mode switched to kd");
                _settingState = SettingState.kd;
            }

            if (js.GetButtonColorTop())
            {
                Console.WriteLine("Mode switched to ki");
                _settingState = SettingState.ki;
            }
            
            switch (_settingState)
            {
                case SettingState.offset:
                    if (js.GetHAT().X!=0 || js.GetHAT().Y!=0)
                    {
                        rollOffset += js.GetHAT().X * 1;
                        pitchOffset += js.GetHAT().Y * 1;
                        Console.WriteLine("Offsets: "+rollOffset+", "+pitchOffset);
                        con.SendCommand(new JcRobotNetworking.Command(5,BitConverter.GetBytes(rollOffset))); // roll
                        con.SendCommand(new JcRobotNetworking.Command(6,BitConverter.GetBytes(pitchOffset))); // pitch
                    }
                    break;
                case SettingState.kp:
                    if (js.GetHAT().X!=0 || js.GetHAT().Y!=0)
                    {
                        kp += js.GetHAT().Y * 1;
                        Console.WriteLine("kp: "+kp);
                        con.SendCommand(new JcRobotNetworking.Command(2,BitConverter.GetBytes(kp))); // roll
                    }
                    break;
                case SettingState.kd:
                    if (js.GetHAT().X!=0 || js.GetHAT().Y!=0)
                    {
                        kd += js.GetHAT().Y * .1f;
                        Console.WriteLine("kd: "+kd);
                        con.SendCommand(new JcRobotNetworking.Command(3,BitConverter.GetBytes(kd))); // roll
                    }
                    break;
                case SettingState.ki:
                    if (js.GetHAT().X!=0 || js.GetHAT().Y!=0)
                    {
                        ki += js.GetHAT().Y * .1f;
                        Console.WriteLine("ki: "+ki);
                        con.SendCommand(new JcRobotNetworking.Command(4,BitConverter.GetBytes(ki))); // roll
                    }
                    break;
            }
            

            
            
            
            
            
            
            Thread.Sleep(200);
            }
            
        }
        
        public void Loop()
        {
            con.SendCommand(new JcRobotNetworking.Command(1,BitConverter.GetBytes(-js.GetThumbstickLeft().Y*1024f)));
            Thread.Sleep(50);
        }
        
    }
}