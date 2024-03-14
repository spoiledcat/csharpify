using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using ImGuiNET;
using SDL2;

internal static unsafe class Util
{
	internal const int StackAllocationSizeLimit = 2048;
	internal static byte* Allocate(int byteCount) => (byte*)Marshal.AllocHGlobal(byteCount);
	internal static int GetUtf8(ReadOnlySpan<char> s, byte* utf8Bytes, int utf8ByteCount)
	{
		if (s.IsEmpty)
		{
			return 0;
		}

		fixed (char* utf16Ptr = s)
		{
			return Encoding.UTF8.GetBytes(utf16Ptr, s.Length, utf8Bytes, utf8ByteCount);
		}
	}
	internal static void Free(byte* ptr) => Marshal.FreeHGlobal((IntPtr)ptr);

}

public class Program {

	[UnmanagedCallersOnly()]
	private static void OnStart() {
		Console.WriteLine("hello");

		//SDL.SDL_SetHint(SDL.SDL_HINT_WINDOWS_DISABLE_THREAD_NAMING, "1");
	}

	[UnmanagedCallersOnly()]
    private static void OnUpdate()
    {
	   Console.WriteLine("Update");
	   //  ImGui.Begin("Hello, world!");
	   //  ImGui.End();
	   ImGui.ShowDemoWindow();
    }

    [DllImport("__Internal")]
    public static unsafe extern byte igBegin(byte* name, byte* p_open, ImGuiWindowFlags flags);

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
