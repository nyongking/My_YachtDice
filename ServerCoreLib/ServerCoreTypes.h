#pragma once

using RefIocpCore = std::shared_ptr<class IocpCore>;
using RefIocpObject = std::shared_ptr<class IocpObject>;
using RefSession = std::shared_ptr<class Session>;
using RefPacketSession = std::shared_ptr<class PacketSession>;
using RefListener = std::shared_ptr<class Listener>;
using RefServerService = std::shared_ptr<class ServerService>;
using RefClientService = std::shared_ptr<class ClientService>;
using RefSendBuffer = std::shared_ptr<class SendBuffer>;
using RefSendBufferChunk = std::shared_ptr<class SendBufferChunk>;
using RefJob = std::shared_ptr<class Job>;
using RefJobQueue = std::shared_ptr<class JobQueue>;