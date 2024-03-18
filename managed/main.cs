using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
using System.Text;
using ImGuiNET;
using SDL2;

public partial class Program {

	[UnmanagedCallersOnly()]
	private static void OnStart() {
		var ret = CallToNative();
		Console.WriteLine($"OnStart: {ret}");
	}


	[UnmanagedCallersOnly()]
	private static void OnUpdate()
	{
		ImGui.ShowDemoWindow();
	}





	/* Controlling how a bool gets marshalled */
	[LibraryImport("__Internal")]
	[return: MarshalUsing(typeof(BoolMarshaller))] internal static partial bool CallToNative();



	[CustomMarshaller(typeof(bool), MarshalMode.Default, typeof(BoolMarshaller))]
	internal static unsafe class BoolMarshaller
	{
		public static byte ConvertToUnmanaged(bool managed)
		=> (byte)(managed ? 1 : 0);

		public static bool ConvertToManaged(byte unmanaged)
		=> unmanaged != 0;
	}
}
