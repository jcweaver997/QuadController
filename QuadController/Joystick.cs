using System;
using System.Collections.Generic;
using System.IO;
using System.Threading;

namespace QuadController
{
    public abstract class Joystick
    {
        private FileStream fs;
        private Thread t;
        public Joystick(string devfile)
        {
            Button = new Dictionary<byte, bool>();
            Axis = new Dictionary<byte, short>();
            fs = new FileStream(devfile, FileMode.Open);
            t = new Thread(Update);
            t.Start();
        }

        ~Joystick()
        {
            fs.Close();
        }

        public static void PrintJoysticks()
        {
            foreach (string filename in Directory.EnumerateFiles("/dev/input"))
            {
                if (filename.Contains("js"))
                {
                    Console.WriteLine(filename.Substring(11));
                }
            }
        }

        public struct Vector2{
            public static Vector2 Zero = new Vector2(0, 0);
            public float X;
            public float Y;
            public Vector2(float x, float y){
                this.X = x;
                this.Y = y;
            }
            public override string ToString()
            {
                return "["+X+", "+Y+"]";
            }
        }

        public virtual Vector2 GetThumbstickLeft() => Vector2.Zero;
        public virtual Vector2 GetThumbstickRight() => Vector2.Zero;
        public virtual bool GetButtonColorTop() => false;
        public virtual bool GetButtonColorBottom() => false;
        public virtual bool GetButtonColorRight() => false;
        public virtual bool GetButtonColorLeft() => false;
        public virtual Vector2 GetHAT() => Vector2.Zero;
        public virtual bool GetButtonBumperLeft() => false;
        public virtual bool GetButtonBumperRight() => false;
        public virtual float GetTriggerLeft() => 0;
        public virtual float GetTriggerRight() => 0;
        public virtual bool GetButtonSelect() => false;
        public virtual bool GetButtonStart() => false;

        private void Update(){
            while(true){
                byte[] buff = new byte[8];
                fs.Read(buff, 0, 8);
                DetectChange(buff);
                Thread.Sleep(1);
            }

        }

        enum STATE : byte { PRESSED = 0x01, RELEASED = 0x00 }
        enum TYPE : byte { AXIS = 0x02, BUTTON = 0x01 }
        enum MODE : byte { CONFIGURATION = 0x80, VALUE = 0x00 }
        protected Dictionary<byte, bool> Button;
        protected Dictionary<byte, short> Axis;


        private void DetectChange(byte[] buff)
        {
            // If configuration
            if (checkBit(buff[6], (byte)MODE.CONFIGURATION))
            {
                if (checkBit(buff[6], (byte)TYPE.AXIS))
                {
                    // Axis configuration, read address and register axis
                    byte key = (byte)buff[7];
                    if (!Axis.ContainsKey(key))
                    {
                        Axis.Add(key, 0);
                        return;
                    }
                }
                else if (checkBit(buff[6], (byte)TYPE.BUTTON))
                {
                    // Button configuration, read address and register button
                    byte key = (byte)buff[7];
                    if (!Button.ContainsKey(key))
                    {
                        Button.Add((byte)buff[7], false);
                        return;
                    }
                }
            }

            // If new button/axis value
            if (checkBit(buff[6], (byte)TYPE.AXIS))
            {
                // Axis value, decode U2 and save to Axis dictionary.
                short value = BitConverter.ToInt16(new byte[2] { buff[4], buff[5] }, 0);
                Axis[(byte)buff[7]] = value;
                return;
            }
            else if (checkBit(buff[6], (byte)TYPE.BUTTON))
            {
                // Bytton value, decode value and save to Button dictionary.
                Button[(byte)buff[7]] = buff[4] == (byte)STATE.PRESSED;
                return;
            }
        }

        private bool checkBit(byte value, byte flag)
        {
            byte c = (byte)(value & flag);
            return c == (byte)flag;
        }
    }
}