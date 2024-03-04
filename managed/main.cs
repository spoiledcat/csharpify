using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

public class Program {

	[UnmanagedCallersOnly()]
	private static void CallMe() {
		Console.WriteLine("hello");
	}

	[UnmanagedCallersOnly()]
    private static void InstanceCall()
    {
    }

    //[MethodImpl(MethodImplOptions.InternalCall)]
    [DllImport ("__Internal")]
    internal static extern int CallingBackToNativeLand(int number);
}

namespace Something.Other {

	[StructLayout(LayoutKind.Sequential)]
	public struct NativeData
	{
		public int intField;
	}

    public class NSClass1 {

        [UnmanagedCallersOnly ()]
        public static void SomeCall(NativeData data)
        {
            Console.WriteLine(data.intField);

            var ret = Program.CallingBackToNativeLand(data.intField);
            Console.WriteLine(ret);
        }

        public class NSClass2 {
            [UnmanagedCallersOnly ()]
            public static unsafe void DeepClassNameCall(NativeData* data)
            {
	            NativeData d = *data;
				Console.WriteLine(d.intField);
            }
        }
    }
}
