#include "abstractclient.h"
#include "protocol.h"
#include "protocol_items.h"
#include <QDebug>

AbstractClient::AbstractClient(QObject *parent)
	: QObject(parent), status(StatusDisconnected)
{
}

AbstractClient::~AbstractClient()
{
}

void AbstractClient::processProtocolItem(ProtocolItem *item)
{
	ProtocolResponse *response = qobject_cast<ProtocolResponse *>(item);
	if (response) {
		CommandContainer *cmdCont = pendingCommands.value(response->getCmdId(), 0);
		if (!cmdCont)
			return;
		
		pendingCommands.remove(cmdCont->getCmdId());
		cmdCont->processResponse(response);
		if (response->getReceiverMayDelete())
			delete response;
		cmdCont->deleteLater();
		
		return;
	}
	
	GenericEvent *genericEvent = qobject_cast<GenericEvent *>(item);
	if (genericEvent) {
		switch (genericEvent->getItemId()) {
			case ItemId_Event_ConnectionClosed: emit connectionClosedEventReceived(static_cast<Event_ConnectionClosed *>(item)); break;
			case ItemId_Event_ServerShutdown: emit serverShutdownEventReceived(static_cast<Event_ServerShutdown *>(item)); break;
			case ItemId_Event_AddToList: emit addToListEventReceived(static_cast<Event_AddToList *>(item)); break;
			case ItemId_Event_RemoveFromList: emit removeFromListEventReceived(static_cast<Event_RemoveFromList *>(item)); break;
			case ItemId_Event_UserJoined: emit userJoinedEventReceived(static_cast<Event_UserJoined *>(item)); break;
			case ItemId_Event_UserLeft: emit userLeftEventReceived(static_cast<Event_UserLeft *>(item)); break;
			case ItemId_Event_ServerMessage: emit serverMessageEventReceived(static_cast<Event_ServerMessage *>(item)); break;
			case ItemId_Event_ListRooms: emit listRoomsEventReceived(static_cast<Event_ListRooms *>(item)); break;
			case ItemId_Event_GameJoined: emit gameJoinedEventReceived(static_cast<Event_GameJoined *>(item)); break;
			case ItemId_Event_Message: emit messageEventReceived(static_cast<Event_Message *>(item)); break;
		}
		if (genericEvent->getReceiverMayDelete())
			delete genericEvent;
		return;
	}

	GameEventContainer *gameEventContainer = qobject_cast<GameEventContainer *>(item);
	if (gameEventContainer) {
		emit gameEventContainerReceived(gameEventContainer);
		if (gameEventContainer->getReceiverMayDelete())
			delete gameEventContainer;
		return;
	}

	RoomEvent *roomEvent = qobject_cast<RoomEvent *>(item);
	if (roomEvent) {
		emit roomEventReceived(roomEvent);
		if (roomEvent->getReceiverMayDelete())
			delete roomEvent;
		return;
	}
}

void AbstractClient::setStatus(const ClientStatus _status)
{
	if (_status != status) {
		status = _status;
		emit statusChanged(_status);
	}
}

void AbstractClient::sendCommand(Command *cmd)
{
	sendCommandContainer(new CommandContainer(QList<Command *>() << cmd));
}
