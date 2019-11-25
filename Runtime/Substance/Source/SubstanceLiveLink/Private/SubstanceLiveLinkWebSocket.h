// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SubstanceLiveLinkWebSocket.h
#pragma once

class FSocket;

enum class ESubstanceLiveLinkWebSocketConnectionStatus
{
	Disconnected,
	DisconnectPending,
	ConnectionPending,
	Connected
};

/**
 * WebSocket implementation for Substance Live Link
 * @note We cannot use the libWebSockets provided by UE4 because they do not work with QtWebSockets
 */
class FSubstanceLiveLinkWebSocket
{
public:
	/**
	 * Constructor
	 */
	FSubstanceLiveLinkWebSocket();

	/**
	 * Destructor
	 */
	~FSubstanceLiveLinkWebSocket();

	/**
	 * Open a connection to the given URL
	 * @param Host the hostname to connect to
	 * @param Port the port to connect to on the host
	 */
	void Connect(const ANSICHAR* Host, int Port);

	/**
	 * Disconnect from existing connection (if valid)
	 */
	void Disconnect();

	/**
	 * Get the connection status
	 * @returns ESubstanceLiveLinkWebSocketConnectionStatus dictating the current connection status
	 */
	ESubstanceLiveLinkWebSocketConnectionStatus GetConnectionStatus() const;

	/**
	 * Send a string message to the receiver
	 * @param Message the string to send to the receiver
	 */
	void SendMessage(const ANSICHAR* Message);

	/**
	 * Send a ping message to the receiver to verify the connection	
	*/
	void Ping();
	
	/**
	 * Update the message pumps and dispatch any received messages
	 */
	void Tick();

	/**
	 * This event is called when a message is received
	 * @param the message we received
	 */
	DECLARE_EVENT_OneParam(FSubstanceLiveLinkWebSocket, FMessageReceived, const ANSICHAR*)
	FMessageReceived& OnMessageReceived();

private:
	/** Typedef for Ansi Strings */
	typedef TArray<ANSICHAR> FAnsiCharArray;

	/**
	 * Internal enumeration for sub connection status state processing
	 */
	enum class ESubConnectionStatus
	{
		ConnectionPendingWaitingResponse,
		ConnectionPendingResponseReceived,
	};

	/**
	 * Websocket Opcode Types
	 */
	enum class EWebSocketOpCode : uint8
	{
		Continuation = 0x0,
		Text = 0x1,
		Binary = 0x2,
		Close = 0x8,
		Ping = 0x9,
		Pong = 0xA
	};

	/**
	 * Structure to hold WebSocket data information
	 */
	struct FWebSocketFrame
	{
		/** Opcode type */
		EWebSocketOpCode OpCode;

		/** Payload Mask */
		uint8 Mask[4];

		/** True if this frame is the final fragment of the message */
		bool FinalFragment;

		/** True if the message uses a mask */
		bool UseMask;

		/** Payload data (unmasked) */
		TArray<uint8> Payload;
	};

private:
	/**
	 * Process incoming messages
	 */
	void DecodeIncomingMessages();

	/**
	 * Try to read a raw line from the connection socket
	 * @param LineOut the buffer to read into
	 * @param LineLength the length of the buffer passed in
	 * @returns true if a message was received
	 */
	bool RecvRawString(ANSICHAR* LineOut, int32 LineLength);

	/**
	 * Add a string to the transmit buffer without any encoding
	 * @param Line the string to transmit
	 */
	void SendRawString(const ANSICHAR* String);

	/**
	 * Transmit Outgoing Messages to Server
	 */
	void TransmitOutgoingMessages();

private:
	/** Internal Socket Object */
	FSocket* Socket;

	/** Current Connection Status */
	ESubstanceLiveLinkWebSocketConnectionStatus ConnectionStatus;

	/** Sub Connection Status for Internal Use */
	ESubConnectionStatus SubConnectionStatus;

	/** Message Received Event */
	FMessageReceived MessageReceivedEvent;

	/** Incoming messages */
	TArray<FWebSocketFrame> IncomingMessages;

	/** Outgoing messages */
	TArray<FWebSocketFrame> OutgoingMessages;

	/** Raw Transmit buffer */
	TArray<uint8> TxBuffer;

	/** Raw Receive Buffer */
	TArray<uint8> RxBuffer;
};
