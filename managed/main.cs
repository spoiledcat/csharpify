using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

public unsafe class Program {

	[UnmanagedCallersOnly ()]
	public static void CallMe() {
		Console.WriteLine("hello");
	}

	[UnmanagedCallersOnly ()]
	public static void CallMe2() {
		Console.WriteLine("hello");
	}


	// [UnmanagedCallersOnly(EntryPoint = "reverse_inplace_ref_ushort")]
    //     public static void ReverseInPlaceUShort(ushort** refInput)
    //     {
    //         if (*refInput == null)
    //             return;

    //         int len = GetLength(*refInput);
    //         var span = new Span<ushort>(*refInput, len);
    //         span.Reverse();
    //     }


	// 	internal static ushort* Reverse(ushort *s)
    //     {
    //         if (s == null)
    //             return null;

    //         int len = GetLength(s);
    //         ushort* ret = (ushort*)Marshal.AllocCoTaskMem((len + 1) * sizeof(ushort));
    //         var span = new Span<ushort>(ret, len);

    //         new Span<ushort>(s, len).CopyTo(span);
    //         span.Reverse();
    //         ret[len] = 0;
    //         return ret;
    //     }

	// private unsafe static int GetLength(byte* input)
    //     {
    //         if (input == null)
    //             return 0;

    //         int len = 0;
    //         while (*input != 0)
    //         {
    //             input++;
    //             len++;
    //         }

    //         return len;
    //     }
}