using System;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
using System.Text;
using ImGuiNET;
using SDL2;

public partial class Program {

	[UnmanagedCallersOnly()]
	private static void OnStart() {
		// called when starting up
		int a = 0;
	}


	[UnmanagedCallersOnly()]
	private static void OnUpdate()
	{
		//ImGui.ShowDemoWindow();

		ImGui.Begin("My Cool Window");

		ImGui.Text("Some text!");

		bool boolWrong = BoolWithoutMarshalling();
		ImGui.Text($"Unmarshalled bool value. Should be 'False', it's '{boolWrong}'??");



		//bool boolRight = ReturningBool();
		//ImGui.Text($"Marshalled bool value. Should be 'False', it's '{boolRight}'");

		//boolRight = BoolMarshaledAsI1();
		//ImGui.Text($"Marshalled as I1 bool value. Should be 'False', it's '{boolRight}'");

		//string val = BoolMarshaledPinvoke();
		//ImGui.Text($"Marshalled Pinvoke bool value. Should be 'False', it's '{val}'");

		//boolRight = ReturningByte1() != 0;
		//ImGui.Text($"Unmarshalled LibraryImport byte value. Should be 'False', it's '{boolRight}'");

		//boolRight = ReturningByte2() != 0;
		//ImGui.Text($"Unmarshalled DllImport byte value. Should be 'False', it's '{boolRight}'");


		ImGui.End();
	}


	// This one is not marshalled so the native side has to take care of it
	// LibraryImport cannot be used with a bool return value because it will
	// refuse (rightly) to generate marshalling information for bools
	// For this example we use a direct pinvoke to see what happens.
	[DllImport("__Internal", EntryPoint = "ReturningBool")]
	internal static extern bool BoolWithoutMarshalling();



	// Marshalling the 8-bit value that the native side is returning into a 32-bit c# bool.
	// LibraryImport looks at the type used by Convert* methods in the marshaller and
	// generates a PInvoke of that type (byte in this case), and then the BoolMarshaller is
	// called to convert the value to the type defined by the CustomMarshaller attribute (bool)
	[LibraryImport("__Internal")]
	[return: MarshalUsing(typeof(BoolMarshaller))] internal static partial bool ReturningBool();

	// Same as above, only without the marshaller but still with LibraryImport. 
	// LibraryImport cannot be used with a bool return value because it will
	// refuse (rightly) to generate marshalling information for bools, so we have to use a byte
	// and the caller has to do the conversion.
	[LibraryImport("__Internal", EntryPoint = "ReturningBool")]
	internal static partial byte ReturningByte1();

	// Same as above, only without the automatic marshaller and using direct pinvoke
	[DllImport("__Internal", EntryPoint = "ReturningBool")]
	internal static extern byte ReturningByte2();

	// Direct pinvoke with DllImport also supports marshalling of types, but it only
	// supports a set of type conversions as defined in the UnmanagedType enum.
	// For more complex conversions, you have to implement the
	// System.Runtime.InteropServices.ICustomMarshaler interface
	[DllImport("__Internal", EntryPoint = "ReturningBool")]
	[return: MarshalAs(UnmanagedType.I1)] internal static extern bool BoolMarshaledAsI1();

	// Custom marshallers with DllImport are only allowed on reference types so we're going to
	// make a reference type just for this example
	[DllImport("__Internal", EntryPoint = "ReturningBool")]
	[return: MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(DllImportBoolMarshaler))] internal static extern string BoolMarshaledPinvoke();




	[UnmanagedCallersOnly]
	public static unsafe void UpdateStructField(MyData* d)
	{
		d->field1 += d->field2;
	}


	[CustomMarshaller(typeof(bool), MarshalMode.Default, typeof(BoolMarshaller))]
	internal static unsafe class BoolMarshaller
	{
		public static byte ConvertToUnmanaged(bool managed)
		=> (byte)(managed ? 1 : 0);

		public static bool ConvertToManaged(byte unmanaged)
		=> unmanaged != 0;
	}

	public class DllImportBoolMarshaler : ICustomMarshaler
	{
		public static ICustomMarshaler GetInstance(string str)
		{
			return new DllImportBoolMarshaler();
		}

		public void CleanUpManagedData(object ManagedObj)
		{
			
		}

		public void CleanUpNativeData(IntPtr pNativeData)
		{
			
		}

		public int GetNativeDataSize()
		{
			return 1;
		}

		public IntPtr MarshalManagedToNative(object ManagedObj)
		{
			return IntPtr.Zero;
		}

		public object MarshalNativeToManaged(IntPtr pNativeData)
		{
			return ((byte) pNativeData) != 0 ? "True" : "False";
		}
	}

	public struct MyData
	{
		public int field1;
		public short field2;
	}
}
