#include "RoomExporter.h"
#include "Utils/BinaryWriter.h"
#include "Utils/MemoryStream.h"
#include "Utils/File.h"
#include <ZRoom/Commands/SetMesh.h>
#include <ZRoom/Commands/SetWind.h>
#include <ZRoom/Commands/SetTimeSettings.h>
#include <ZRoom/Commands/SetSkyboxModifier.h>
#include <ZRoom/Commands/SetSoundSettings.h>
#include <ZRoom/Commands/SetCameraSettings.h>
#include <ZRoom/Commands/SetRoomBehavior.h>
#include <ZRoom/Commands/SetCsCamera.h>
#include <ZRoom/Commands/SetRoomList.h>
#include <ZRoom/Commands/SetCollisionHeader.h>
#include <ZRoom/Commands/SetEntranceList.h>
#include <ZRoom/Commands/SetSpecialObjects.h>
#include <ZRoom/Commands/SetStartPositionList.h>
#include <ZRoom/Commands/SetSkyboxSettings.h>
#include <ZRoom/Commands/SetLightingSettings.h>
#include <ZRoom/Commands/SetLightList.h>
#include <ZRoom/Commands/SetEchoSettings.h>
#include <ZRoom/Commands/SetObjectList.h>
#include <ZRoom/Commands/SetAlternateHeaders.h>
#include "CollisionExporter.h"
#include "DisplayListExporter.h"
#include "OTRResource.h"
#include <Globals.h>
#include <ZRoom/Commands/SetExitList.h>

void OTRExporter_Room::Save(ZResource* res, fs::path outPath, BinaryWriter* writer)
{
	ZRoom* room = (ZRoom*)res;

	writer->Write((uint8_t)Endianess::Little);
	writer->Write((uint32_t)OtrLib::ResourceType::OTRRoom);
	writer->Write((uint32_t)OtrLib::OTRVersion::Deckard);
	writer->Write((uint64_t)0xDEADBEEFDEADBEEF); // id

	writer->Write((uint32_t)room->commands.size());

	for (int i = 0; i < room->commands.size(); i++)
	{
		ZRoomCommand* cmd = room->commands[i];
				
		writer->Write((uint32_t)cmd->cmdID);

		switch (cmd->cmdID)
		{
		case RoomCommand::SetActorList:
		{
			SetActorList* cmdSetActorList = (SetActorList*)cmd;

			writer->Write((uint32_t)cmdSetActorList->actors.size());

			for (const ActorSpawnEntry& entry : cmdSetActorList->actors)
			{
				writer->Write(entry.actorNum);
				writer->Write(entry.posX);
				writer->Write(entry.posY);
				writer->Write(entry.posZ);
				writer->Write(entry.rotX);
				writer->Write(entry.rotY);
				writer->Write(entry.rotZ);
				writer->Write(entry.initVar);
			}
		}
			break;
		case RoomCommand::SetWind:
		{
			SetWind* cmdSetWind = (SetWind*)cmd;

			writer->Write(cmdSetWind->windWest); // 0x04
			writer->Write(cmdSetWind->windVertical); // 0x05
			writer->Write(cmdSetWind->windSouth); // 0x06
			writer->Write(cmdSetWind->clothFlappingStrength); // 0x07
		}
			break;
		case RoomCommand::SetTimeSettings:
		{
			SetTimeSettings* cmdTime = (SetTimeSettings*)cmd;

			writer->Write(cmdTime->hour); // 0x04
			writer->Write(cmdTime->min); // 0x05
			writer->Write(cmdTime->unk); // 0x06
		}
		break;
		case RoomCommand::SetSkyboxModifier:
		{
			SetSkyboxModifier* cmdSkybox = (SetSkyboxModifier*)cmd;

			writer->Write(cmdSkybox->disableSky); // 0x04
			writer->Write(cmdSkybox->disableSunMoon); // 0x05
		}
		break;
		case RoomCommand::SetEchoSettings:
		{
			SetEchoSettings* cmdEcho = (SetEchoSettings*)cmd;

			writer->Write((uint8_t)cmdEcho->echo); // 0x07
		}
		break;
		case RoomCommand::SetSoundSettings:
		{
			SetSoundSettings* cmdSound = (SetSoundSettings*)cmd;

			writer->Write((uint8_t)cmdSound->reverb); // 0x01

			writer->Write(cmdSound->nightTimeSFX); // 0x06
			writer->Write(cmdSound->musicSequence); // 0x07
		}
		break;
		case RoomCommand::SetSkyboxSettings:
		{
			SetSkyboxSettings* cmdSkybox = (SetSkyboxSettings*)cmd;

			writer->Write((uint8_t)cmdSkybox->unk1); // 0x01
			writer->Write((uint8_t)cmdSkybox->skyboxNumber); // 0x04
			writer->Write((uint8_t)cmdSkybox->cloudsType); // 0x05
			writer->Write((uint8_t)cmdSkybox->isIndoors); // 0x06
		}
		break;
		case RoomCommand::SetRoomBehavior:
		{
			SetRoomBehavior* cmdRoom = (SetRoomBehavior*)cmd;

			writer->Write((uint8_t)cmdRoom->gameplayFlags); // 0x01
			writer->Write(cmdRoom->gameplayFlags2); // 0x04
		}
		break;
		case RoomCommand::SetCsCamera:
		{
			SetCsCamera* cmdCsCam = (SetCsCamera*)cmd;

			writer->Write((uint8_t)cmdCsCam->cameras.size()); // 0x01
			writer->Write(cmdCsCam->segmentOffset); // 0x04

			for (int i = 0; i < cmdCsCam->cameras.size(); i++)
			{
				// OTRTODO: FINISH THIS...
			}
		}
		break;
		case RoomCommand::SetMesh:
		{
			SetMesh* cmdMesh = (SetMesh*)cmd;
			
			writer->Write((uint8_t)cmdMesh->data); // 0x01
			writer->Write(cmdMesh->meshHeaderType);

			if (cmdMesh->meshHeaderType == 0 || cmdMesh->meshHeaderType == 2)
			{
				PolygonType2* poly = (PolygonType2*)cmdMesh->polyType.get();

				writer->Write(poly->num);

				for (int i = 0; i < poly->num; i++)
				{
					WritePolyDList(writer, room, &poly->polyDLists[i]);
				}
			}
			else if (cmdMesh->meshHeaderType == 1)
			{
				PolygonType1* poly = (PolygonType1*)cmdMesh->polyType.get();

				// OTRTODO: FINISH THIS

				//writer->Write(poly->format);
				//writer->Write(poly->dlist);

				/*if (poly->format == 2)
				{
					writer->Write(poly->count);
					writer->Write(poly->list);
				}

				if (poly->dlist != 0)
				{
					WritePolyDList(writer, &poly->polyDLists[0]);
				}*/
			}
		}
		break;
		case RoomCommand::SetCameraSettings:
		{
			SetCameraSettings* cmdCam = (SetCameraSettings*)cmd;

			writer->Write((uint8_t)cmdCam->cameraMovement); // 0x01
			writer->Write(cmdCam->mapHighlight); // 0x04
		}
		break;
		case RoomCommand::SetLightList:
		{
			SetLightList* cmdLight = (SetLightList*)cmd;

			writer->Write((uint32_t)cmdLight->lights.size());

			for (int i = 0; i < cmdLight->lights.size(); i++)
			{
				writer->Write(cmdLight->lights[i].type);
				writer->Write(cmdLight->lights[i].x);
				writer->Write(cmdLight->lights[i].y);
				writer->Write(cmdLight->lights[i].z);
				writer->Write(cmdLight->lights[i].r);
				writer->Write(cmdLight->lights[i].g);
				writer->Write(cmdLight->lights[i].b);
				writer->Write(cmdLight->lights[i].drawGlow);
				writer->Write(cmdLight->lights[i].radius);
			}
		}
			break;
		case RoomCommand::SetLightingSettings:
		{
			SetLightingSettings* cmdLight = (SetLightingSettings*)cmd;

			writer->Write((uint32_t)cmdLight->settings.size()); // 0x01

			for (const LightingSettings& setting : cmdLight->settings)
			{
				writer->Write(setting.ambientClrR);
				writer->Write(setting.ambientClrG);
				writer->Write(setting.ambientClrB);

				writer->Write(setting.diffuseClrA_R);
				writer->Write(setting.diffuseClrA_G);
				writer->Write(setting.diffuseClrA_B);

				writer->Write(setting.diffuseDirA_X);
				writer->Write(setting.diffuseDirA_Y);
				writer->Write(setting.diffuseDirA_Z);

				writer->Write(setting.diffuseClrB_R);
				writer->Write(setting.diffuseClrB_G);
				writer->Write(setting.diffuseClrB_B);

				writer->Write(setting.diffuseDirB_X);
				writer->Write(setting.diffuseDirB_Y);
				writer->Write(setting.diffuseDirB_Z);

				writer->Write(setting.fogClrR);
				writer->Write(setting.fogClrG);
				writer->Write(setting.fogClrB);

				writer->Write(setting.unk);
				writer->Write(setting.drawDistance);
			}
		}
		break;
		case RoomCommand::SetRoomList:
		{
			SetRoomList* cmdRoom = (SetRoomList*)cmd;

			writer->Write((uint32_t)cmdRoom->romfile->numRooms); // 0x01

			for (int i = 0;i < cmdRoom->romfile->numRooms; i++)
			{
				std::string roomName = StringHelper::Sprintf("%s\\%s_room_%i", (StringHelper::Split(room->GetName(), "_")[0] + "_scene").c_str(), StringHelper::Split(room->GetName(), "_scene")[0].c_str(), i);
				writer->Write(roomName);
			}
		}
		break;
		case RoomCommand::SetCollisionHeader:
		{
			SetCollisionHeader* cmdCollHeader = (SetCollisionHeader*)cmd;

			Declaration* colHeaderDecl = room->parent->GetDeclaration(cmdCollHeader->segmentOffset);
			std::string path = StringHelper::Sprintf("%s\\%s", (StringHelper::Split(room->GetName(), "_")[0] + "_scene").c_str(), colHeaderDecl->varName.c_str());
			writer->Write(path);
		}
		break;
		case RoomCommand::SetEntranceList:
		{
			SetEntranceList* cmdEntrance = (SetEntranceList*)cmd;

			writer->Write((uint32_t)cmdEntrance->entrances.size());

			for (EntranceEntry entry : cmdEntrance->entrances)
			{
				writer->Write((uint8_t)entry.startPositionIndex);
				writer->Write((uint8_t)entry.roomToLoad);
			}
		}
		break;
		case RoomCommand::SetSpecialObjects:
		{
			SetSpecialObjects* cmdSpecObj = (SetSpecialObjects*)cmd;

			writer->Write((uint8_t)cmdSpecObj->elfMessage); // 0x01
			writer->Write((uint16_t)cmdSpecObj->globalObject); // 0x06
		}
		break;
		case RoomCommand::SetStartPositionList:
		{
			SetStartPositionList* cmdStartPos = (SetStartPositionList*)cmd;

			uint32_t baseStreamEnd = writer->GetStream().get()->GetLength();

			writer->Write((uint32_t)cmdStartPos->actors.size()); // 0x01
			
			for (const ActorSpawnEntry& entry : cmdStartPos->actors)
			{
				writer->Write(entry.actorNum);
				writer->Write(entry.posX);
				writer->Write(entry.posY);
				writer->Write(entry.posZ);
				writer->Write(entry.rotX);
				writer->Write(entry.rotY);
				writer->Write(entry.rotZ);
				writer->Write(entry.initVar);
			}
		}
		break;
		case RoomCommand::SetAlternateHeaders:
		{
			SetAlternateHeaders* cmdHeaders = (SetAlternateHeaders*)cmd;

			writer->Write((uint32_t)cmdHeaders->headers.size());

			for (int i = 0; i < cmdHeaders->headers.size(); i++)
			{
				uint32_t seg = cmdHeaders->headers[i] & 0xFFFFFFFF;
				std::string headerName = "";
				bool foundDecl = Globals::Instance->GetSegmentedPtrName(seg, room->parent, "", headerName);

				writer->Write(OTRExporter_DisplayList::GetPathToRes(room, headerName));
			}
		}
			break;
		case RoomCommand::SetExitList:
		{
			SetExitList* cmdExit = (SetExitList*)cmd;

			writer->Write((uint32_t)cmdExit->exits.size());

			for (int i = 0; i < cmdExit->exits.size(); i++)
				writer->Write(cmdExit->exits[i]);
		}
			break;
		case RoomCommand::SetObjectList:
		{
			SetObjectList* cmdSetObjectList = (SetObjectList*)cmd;

			writer->Write((uint32_t)cmdSetObjectList->objects.size());

			for (int i = 0; i < cmdSetObjectList->objects.size(); i++)
				writer->Write(cmdSetObjectList->objects[i]);
		}
			break;
		case RoomCommand::EndMarker:
			break;
		default:
			printf("UNIMPLEMENTED COMMAND: 0x%02X\n", (int)cmd->cmdID);

			break;
		}
	}
}

void OTRExporter_Room::WritePolyDList(BinaryWriter* writer, ZRoom* room, PolygonDlist* dlist)
{
	writer->Write(dlist->polyType);

	switch (dlist->polyType)
	{
	case 2:
		writer->Write(dlist->x);
		writer->Write(dlist->y);
		writer->Write(dlist->z);
		writer->Write(dlist->unk_06);
	default:
		if (dlist->opaDList != nullptr)
			writer->Write(StringHelper::Sprintf("%s\\%s", (StringHelper::Split(room->GetName(), "_")[0] + "_scene").c_str(), dlist->opaDList->GetName().c_str()));
		else
			writer->Write("");

		if (dlist->xluDList != nullptr)
			writer->Write(StringHelper::Sprintf("%s\\%s", (StringHelper::Split(room->GetName(), "_")[0] + "_scene").c_str(), dlist->xluDList->GetName().c_str()));
		else
			writer->Write("");
		break;
	}
}
