// Copyright 2017 Allegorithmic Inc. All rights reserved.
// File: SubstanceLiveLinkWebSocket.cpp
#include "SubstanceLiveLinkWebSocket.h"
#include "SubstanceLiveLinkPrivatePCH.h"
#include "IPAddress.h"
#include "Sockets.h"
#include "SocketSubsystem.h"

/**
 */
FSubstanceLiveLinkWebSocket::FSubstanceLiveLinkWebSocket()
	: Socket(nullptr)
	, ConnectionStatus(ESubstanceLiveLinkWebSocketConnectionStatus::Disconnected)
{
}

FSubstanceLiveLinkWebSocket::~FSubstanceLiveLinkWebSocket()
{
	Disconnect();
}

void FSubstanceLiveLinkWebSocket::Connect(const ANSICHAR* Host, int Port)
{
	Disconnect();

	ConnectionStatus = ESubstanceLiveLinkWebSocketConnectionStatus::ConnectionPending;
	SubConnectionStatus = ESubConnectionStatus::ConnectionPendingWaitingResponse;

	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get();
	Socket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("SubstanceLiveLinkWebSocket"));
	Socket->SetNonBlocking(true);

	TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr(0, 0);

	if (SE_NO_ERROR != SocketSubsystem->GetHostByName(Host, *Addr))
	{
		UE_LOG(LogSubstanceLiveLink, Warning, TEXT("Unable to resolve Host %s for Web Socket Connection"), UTF8_TO_TCHAR(Host));
		ConnectionStatus = ESubstanceLiveLinkWebSocketConnectionStatus::Disconnected;
		Disconnect();
		return;
	}

	Addr->SetPort(Port);

	if (Socket->Connect(*Addr))
	{
		SendRawString("GET / HTTP/1.1\r\n");
		SendRawString(TCHAR_TO_UTF8(*FString::Printf(TEXT("Host: %s:%d\r\n"), UTF8_TO_TCHAR(Host), Port)));
		SendRawString("Upgrade: websocket\r\n");
		SendRawString("Connection: Upgrade\r\n");
		SendRawString("Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==\r\n");
		SendRawString("Sec-WebSocket-Version: 13\r\n");
		SendRawString("\r\n");
	}
}

void FSubstanceLiveLinkWebSocket::Disconnect()
{
	if (Socket != nullptr)
	{
		ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get();
		SocketSubsystem->DestroySocket(Socket);

		Socket = nullptr;
	}

	OutgoingMessages.Empty();
	IncomingMessages.Empty();
	TxBuffer.Empty();
	RxBuffer.Empty();

	ConnectionStatus = ESubstanceLiveLinkWebSocketConnectionStatus::Disconnected;
}

ESubstanceLiveLinkWebSocketConnectionStatus FSubstanceLiveLinkWebSocket::GetConnectionStatus() const
{
	return ConnectionStatus;
}

void FSubstanceLiveLinkWebSocket::SendMessage(const ANSICHAR* Message)
{
	const uint8 ClientMask[] = { 0x1, 0x2, 0x3, 0x4 }; //lame and unsecure!

	FWebSocketFrame Frame;
	Frame.OpCode = EWebSocketOpCode::Text;
	Frame.FinalFragment = true;
	Frame.UseMask = true;
	FMemory::Memcpy(Frame.Mask, ClientMask, sizeof(ClientMask));

	Frame.Payload.Append(reinterpret_cast<const uint8*>(Message), FCStringAnsi::Strlen(Message));

	OutgoingMessages.Add(MoveTemp(Frame));
}

void FSubstanceLiveLinkWebSocket::Ping()
{
	FWebSocketFrame Frame;
	Frame.OpCode = EWebSocketOpCode::Ping;
	Frame.FinalFragment = true;
	Frame.UseMask = false;

	OutgoingMessages.Add(MoveTemp(Frame));
}

void FSubstanceLiveLinkWebSocket::Tick()
{
	if (Socket == nullptr)
	{
		return;
	}

	//empty transmit buffer
	if (TxBuffer.Num())
	{
		int32 BytesSent = 0;

		if (Socket->Send(TxBuffer.GetData(), TxBuffer.Num(), BytesSent))
		{
			TxBuffer.RemoveAt(0, BytesSent);
		}
		else
		{
			UE_LOG(LogSubstanceLiveLink, Warning, TEXT("Unable to send data along socket, closing connection"));
			Disconnect();
			return;
		}
	}

	//fill receive buffer
	uint8 Buffer[256];
	int32 BytesRead;

	if (Socket->Recv(Buffer, sizeof(Buffer), BytesRead, ESocketReceiveFlags::None) && BytesRead > 0)
	{
		RxBuffer.Append(Buffer, BytesRead);
	}

	//update logic based on our connection status
	switch (ConnectionStatus)
	{
	case ESubstanceLiveLinkWebSocketConnectionStatus::ConnectionPending:
		{
			if (SubConnectionStatus == ESubConnectionStatus::ConnectionPendingWaitingResponse)
			{
				ANSICHAR LineOut[256];

				if (RecvRawString(LineOut, sizeof(LineOut)))
				{
					if (!FString(UTF8_TO_TCHAR(LineOut)).StartsWith("HTTP/1.1 101"))
					{
						UE_LOG(LogSubstanceLiveLink, Warning, TEXT("Invalid HTTP Code Returned by Painter, closing connection"));
						Disconnect();
						return;
					}
					else
					{
						SubConnectionStatus = ESubConnectionStatus::ConnectionPendingResponseReceived;
					}
				}
			}
			else if (SubConnectionStatus == ESubConnectionStatus::ConnectionPendingResponseReceived)
			{
				ANSICHAR LineOut[256];

				if (RecvRawString(LineOut, sizeof(LineOut)))
				{
					//we don't care about any of the HTTP Headers, just connect us at the end
					if (LineOut[0] == '\r' && LineOut[1] == '\n')
					{
						ConnectionStatus = ESubstanceLiveLinkWebSocketConnectionStatus::Connected;
					}
				}
			}
		}
		break;
	case ESubstanceLiveLinkWebSocketConnectionStatus::Connected:
		{
			//convert raw data streams into web socket frames
			TransmitOutgoingMessages();
			DecodeIncomingMessages();

			//process messages
			TArray<const FWebSocketFrame*> RemoveMessages;
			TArray<const FWebSocketFrame*> Fragments;

			//Store incoming messages in temp array, just in case broadcast functions queue up more messages
			TArray<FWebSocketFrame> IncomingMessagesTemp(MoveTemp(IncomingMessages));

			for (const auto& Message : IncomingMessagesTemp)
			{
				Fragments.Add(&Message);

				if (Message.FinalFragment)
				{
					RemoveMessages.Append(Fragments);

					switch (Message.OpCode)
					{
					case EWebSocketOpCode::Text:
						{
							//Construct Payload
							FAnsiCharArray Payload;

							for (const auto& Fragment : Fragments)
							{
								Payload.Append(reinterpret_cast<const ANSICHAR*>(Fragment->Payload.GetData()), Fragment->Payload.Num());
							}

							//Add null terminator
							Payload.Add(0);

							MessageReceivedEvent.Broadcast(Payload.GetData());
						}
						break;
					case EWebSocketOpCode::Close:
						{
							Disconnect();
						}
						break;
					case EWebSocketOpCode::Pong:
						{

						}
						break;
					default:
						{
							UE_LOG(LogSubstanceLiveLink, Warning, TEXT("Unsupported OpCode provided by Painter, closing connection"));
							Disconnect();
							return;
						}
						break;
					}

					Fragments.Empty();
				}
			}

			//remove all messages we processed
			IncomingMessagesTemp.RemoveAll([&RemoveMessages](const FWebSocketFrame & Frame)
			{
				return RemoveMessages.Contains(&Frame);
			});

			//Append IncomingMessages to temp object in case new ones have come in
			IncomingMessagesTemp.Append(MoveTemp(IncomingMessages));

			//Swap arrays back to object storage
			IncomingMessages = MoveTemp(IncomingMessagesTemp);
		}
		break;
	default:
		break;
	}
}

FSubstanceLiveLinkWebSocket::FMessageReceived& FSubstanceLiveLinkWebSocket::OnMessageReceived()
{
	return MessageReceivedEvent;
}

bool FSubstanceLiveLinkWebSocket::RecvRawString(ANSICHAR* LineOut, int32 LineLength)
{
	for (int32 i = 0; i < RxBuffer.Num() - 1; i++)
	{
		if (RxBuffer[i] == '\r' && RxBuffer[i + 1] == '\n')
		{
			if (i + 2 >= LineLength)
			{
				UE_LOG(LogSubstanceLiveLink, Warning, TEXT("RecvRawString has a string of length %d, but LineOut is undersized at %d bytes"), i + 2, LineLength);
				return false;
			}

			FMemory::Memcpy(LineOut, RxBuffer.GetData(), i + 2);
			LineOut[i + 2] = 0;

			RxBuffer.RemoveAt(0, i + 2);
			return true;
		}
	}

	//no string available
	return false;
}

void FSubstanceLiveLinkWebSocket::SendRawString(const ANSICHAR* Line)
{
	TxBuffer.Append(reinterpret_cast<const uint8*>(Line), FCStringAnsi::Strlen(Line));
}

void FSubstanceLiveLinkWebSocket::DecodeIncomingMessages()
{
	//WebSocket Header Documentation: https://tools.ietf.org/html/rfc6455#page-27
	while (RxBuffer.Num() > 2)
	{
		const uint8* Data = RxBuffer.GetData();

		FWebSocketFrame Frame;
		Frame.OpCode = (EWebSocketOpCode)(Data[0] & 0x0F);
		Frame.FinalFragment = ((Data[0] & 0x80) == 0x80) ? true : false;
		Frame.UseMask = ((Data[1] & 0x80) == 0x80) ? true : false;
		int32 PayloadToken = Data[1] & 0x7F;

		//Calculate Header Size
		int32 HeaderSize = 2;
		int32 MaskOffset = 2;

		if (PayloadToken == 126)
		{
			HeaderSize += 2;
			MaskOffset += 2;
		}
		else if (PayloadToken == 127)
		{
			HeaderSize += 8;
			MaskOffset += 8;
		}

		HeaderSize += Frame.UseMask ? 4 : 0;

		//Sanity check
		if (RxBuffer.Num() < HeaderSize)
		{
			return;
		}

		//Calculate Payload Size
		int32 ActualPayloadSize = PayloadToken;

		if (PayloadToken == 126)
		{
			ActualPayloadSize = (uint32)Data[2] << 8 | (uint32)Data[3];
		}
		else if (PayloadToken == 127)
		{
			UE_LOG(LogSubstanceLiveLink, Warning, TEXT("Payloads over 64K not supported from Painter, closing connection"));
			Disconnect();
			return;
		}

		if (Frame.UseMask)
		{
			Frame.Mask[0] = Data[MaskOffset];
			Frame.Mask[1] = Data[MaskOffset + 1];
			Frame.Mask[2] = Data[MaskOffset + 2];
			Frame.Mask[3] = Data[MaskOffset + 3];
		}

		//Verify we have the entire message in the Rx Buffer
		if (RxBuffer.Num() < ActualPayloadSize + HeaderSize)
		{
			return;
		}

		//Unmask payload
		if (Frame.UseMask)
		{
			for (int32 i = 0; i < ActualPayloadSize; i++)
				RxBuffer[i + HeaderSize] ^= Frame.Mask[i & 3];
		}

		Frame.Payload.Append(RxBuffer.GetData() + HeaderSize, ActualPayloadSize);
		RxBuffer.RemoveAt(0, HeaderSize + ActualPayloadSize);

		IncomingMessages.Add(MoveTemp(Frame));
	}
}

void FSubstanceLiveLinkWebSocket::TransmitOutgoingMessages()
{
	for (auto& Frame : OutgoingMessages)
	{
		uint8 Header[14];
		int32 MaskOffset = 0;

		Header[0] = static_cast<uint8>(Frame.OpCode);
		Header[0] |= Frame.FinalFragment ? 0x80 : 0x0;

		//encode message length as needed
		if (Frame.Payload.Num() < 126)
		{
			Header[1] = Frame.Payload.Num();
			Header[1] |= (Frame.UseMask) ? 0x80 : 0x00;
			MaskOffset = 2;
		}
		else if (Frame.Payload.Num() < 65536)
		{
			Header[1] = 126;
			Header[1] |= (Frame.UseMask) ? 0x80 : 0x00;
			Header[2] = (Frame.Payload.Num() >> 8) & 0xFF;
			Header[3] = Frame.Payload.Num() & 0xFF;
			MaskOffset = 4;
		}
		else
		{
			UE_LOG(LogSubstanceLiveLink, Error, TEXT("We don't support sending messages over 64K, closing connection"));
			Disconnect();
			return;
		}

		int32 HeaderSize = MaskOffset;

		//encode mask
		if (Frame.UseMask)
		{
			Header[MaskOffset] = Frame.Mask[0];
			Header[MaskOffset + 1] = Frame.Mask[1];
			Header[MaskOffset + 2] = Frame.Mask[2];
			Header[MaskOffset + 3] = Frame.Mask[3];

			//encode mask into message
			for (int32 i = 0; i < Frame.Payload.Num(); i++)
				Frame.Payload[i] ^= Frame.Mask[i & 3];

			HeaderSize += 4;
		}

		//add to transmit buffer
		TxBuffer.Append(Header, HeaderSize);
		TxBuffer.Append(reinterpret_cast<const uint8*>(Frame.Payload.GetData()), Frame.Payload.Num());
	}

	OutgoingMessages.Empty();
}
