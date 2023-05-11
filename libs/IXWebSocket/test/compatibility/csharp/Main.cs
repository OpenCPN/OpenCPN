//
// Main.cs
// Author: Benjamin Sergeant
// Copyright (c) 2020 Machine Zone, Inc. All rights reserved.
//
// In a different terminal, start a push server:
// $ ws push_server -q
//
// $ dotnet run
// messages received per second: 145157
// messages received per second: 141405
// messages received per second: 152202
// messages received per second: 157149
// messages received per second: 157673
// messages received per second: 153594
// messages received per second: 157830
// messages received per second: 158422
//

using System;
using System.Net.WebSockets;
using System.Threading;
using System.Threading.Tasks;

public class DevNullClientCli
{
    private static int receivedMessage = 0;

    public static async Task<byte[]> ReceiveAsync(ClientWebSocket ws, CancellationToken token)
    {
        int bufferSize = 8192; // 8K
        var buffer = new byte[bufferSize];
        var offset = 0;
        var free = buffer.Length;

        while (true)
        {
            var result = await ws.ReceiveAsync(new ArraySegment<byte>(buffer, offset, free), token).ConfigureAwait(false);

            offset += result.Count;
            free -= result.Count;
            if (result.EndOfMessage) break;

            if (free == 0)
            {
                // No free space
                // Resize the outgoing buffer
                var newSize = buffer.Length + bufferSize;

                var newBuffer = new byte[newSize];
                Array.Copy(buffer, 0, newBuffer, 0, offset);
                buffer = newBuffer;
                free = buffer.Length - offset;
            }
        }

        return buffer;
    }

    private static void OnTimedEvent(object source, EventArgs e)
    {
        Console.WriteLine($"messages received per second: {receivedMessage}");
        receivedMessage = 0;
    }

    public static async Task ReceiveMessagesAsync(string url)
    {
        var ws = new ClientWebSocket();

        System.Uri uri = new System.Uri(url);
        var cancellationToken = CancellationToken.None;

        try 
        {
            await ws.ConnectAsync(uri, cancellationToken).ConfigureAwait(false);
            while (true)
            {
                var data = await DevNullClientCli.ReceiveAsync(ws, cancellationToken);
                receivedMessage += 1;
            }
        }
        catch (System.Net.WebSockets.WebSocketException e)
        {
            Console.WriteLine($"WebSocket error: {e}");
            return;
        }
    }

    public static async Task Main()
    {
        var timer = new System.Timers.Timer(1000);
        timer.Elapsed += OnTimedEvent;
        timer.Enabled = true;
        timer.Start();

        var url = "ws://localhost:8008";
        await ReceiveMessagesAsync(url);
    }
}
