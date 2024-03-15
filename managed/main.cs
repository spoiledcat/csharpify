using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using ImGuiNET;
using SDL2;

public partial class Program {

	ref struct RefData {
		public byte IsValid;
	}

	[UnmanagedCallersOnly()]
	private static void OnStart() {
		Console.WriteLine("hello");

		send_utf16("A UTF16 string?");
		send_utf8("A UTF8 string?");
	}

	[UnmanagedCallersOnly()]
    private static void OnUpdate()
    {
	   ImGui.ShowDemoWindow();
    }

    //[MethodImpl(MethodImplOptions.InternalCall)]
    [DllImport ("__Internal")]
    internal static extern int CallingBackToNativeLand(int number);

	[UnmanagedCallersOnly()]
	static void ValidateData(RefData data){ 

	}

	[LibraryImport("__Internal", StringMarshalling = StringMarshalling.Utf16)]
	internal static partial void send_utf16(string str);


	[LibraryImport("__Internal", StringMarshalling = StringMarshalling.Utf8)]
	internal static partial void send_utf8(string str);
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
            Console.WriteLine($"Initial value {data.intField}");

            var ret = Program.CallingBackToNativeLand(data.intField);

            Console.WriteLine($"Received {ret}");
        }
    }
}
