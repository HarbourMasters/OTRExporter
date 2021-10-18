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
#include <ZRoom/Commands/SetMesh.h>
#include <ZRoom/Commands/SetRoomBehavior.h>
#include <ZRoom/Commands/SetCsCamera.h>
#include <ZRoom/Commands/SetRoomList.h>
#include <ZRoom/Commands/SetCollisionHeader.h>
#include <ZRoom/Commands/SetEntranceList.h>
#include <ZRoom/Commands/SetSpecialObjects.h>
#include <ZRoom/Commands/SetStartPositionList.h>
#include <ZRoom/Commands/SetSkyboxSettings.h>
#include <ZRoom/Commands/SetLightingSettings.h>
#include <ZRoom/Commands/SetEchoSettings.h>
#include <CollisionExporter.h>

void ExporterExample_Room::Save(ZResource* res, fs::path outPath, BinaryWriter* writer)
{
	ZRoom* room = (ZRoom*)res;

	//writer->Write(room->commands.size());

	for (int i = 0; i < room->commands.size(); i++)
	{
		ZRoomCommand* cmd = room->commands[i];
				
		writer->Write((uint8_t)cmd->cmdID);

		switch (cmd->cmdID)
		{
		case RoomCommand::SetActorList:
		{
			SetActorList* cmdSetActorList = (SetActorList*)cmd;

			writer->Write(cmdSetActorList->actors.size());

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
				// TODO: FINISH THIS...
			}
		}
		break;
		case RoomCommand::SetMesh:
		{
			SetMesh* cmdMesh = (SetMesh*)cmd;
			 
			int baseStreamEnd = writer->GetStream().get()->GetLength();

			writer->Write((uint8_t)cmdMesh->data); // 0x01
			writer->Write(cmdMesh->meshHeaderType);

			if (cmdMesh->meshHeaderType == 0 || cmdMesh->meshHeaderType == 2)
			{
				PolygonType2* poly = (PolygonType2*)cmdMesh->polyType.get();

				//writer->Write(poly->num);
				//writer->Write(poly->start);
				//writer->Write(poly->end);

				for (int i = 0; i < poly->num; i++)
				{
					//WritePolyDList(writer, &poly->polyDLists[i]);
				}
			}
			else if (cmdMesh->meshHeaderType == 1)
			{
				PolygonType1* poly = (PolygonType1*)cmdMesh->polyType.get();

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

			//writer->Seek(oldOffset, SeekOffsetType::Start);
		}
		break;
		case RoomCommand::SetCameraSettings:
		{
			SetCameraSettings* cmdCam = (SetCameraSettings*)cmd;

			writer->Write((uint8_t)cmdCam->cameraMovement); // 0x01
			writer->Write(cmdCam->mapHighlight); // 0x04
		}
		break;
		case RoomCommand::SetLightingSettings:
		{
			SetLightingSettings* cmdLight = (SetLightingSettings*)cmd;

			writer->Write((uint8_t)cmdLight->settings.size()); // 0x01
			writer->Write(cmdLight->segmentOffset); // 0x04

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

			// TODO: FINISH
			
			//writer->Write((uint8_t)cmdRoom->rooms.size()); // 0x01

			//for (RoomEntry entry : cmdRoom->rooms)
			//{
			//	writer->Write(entry.virtualAddressStart);
			//	writer->Write(entry.virtualAddressEnd);
			//}
		}
		break;
		case RoomCommand::SetCollisionHeader:
		{
			SetCollisionHeader* cmdCollHeader = (SetCollisionHeader*)cmd;

			ExporterExample_Collision colExp = ExporterExample_Collision();
			//colExp.Save(cmdCollHeader->collisionHeader, outPath, writer);
		}
		break;
		case RoomCommand::SetEntranceList:
		{
			SetEntranceList* cmdEntrance = (SetEntranceList*)cmd;

			//uint32_t baseStreamEnd = writer->GetStream().get()->GetLength();
			//writer->Write(baseStreamEnd); // 0x04

			//uint32_t oldOffset = writer->GetBaseAddress();
			//writer->Seek(baseStreamEnd, SeekOffsetType::Start);

			writer->Write(cmdEntrance->entrances.size());

			for (EntranceEntry entry : cmdEntrance->entrances)
			{
				writer->Write((uint8_t)entry.startPositionIndex);
				writer->Write((uint8_t)entry.roomToLoad);
			}

			//writer->Seek(oldOffset, SeekOffsetType::Start);
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

			writer->Write(cmdStartPos->actors.size()); // 0x01
			//writer->Write(baseStreamEnd); // 0x04

			//uint32_t oldOffset = writer->GetBaseAddress();
			//writer->Seek(baseStreamEnd, SeekOffsetType::Start);

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

			//writer->Seek(oldOffset, SeekOffsetType::Start);
		}
		break;
		case RoomCommand::EndMarker:
		{
		}
		break;
		default:
			printf("UNIMPLEMENTED COMMAND: %i\n", (int)cmd->cmdID);

			break;
		}
	}

	//writer->Close();
	//File::WriteAllBytes(StringHelper::Sprintf("%s", res->GetName().c_str()), memStream->ToVector());
}

void ExporterExample_Room::WritePolyDList(BinaryWriter* writer, PolygonDlist* dlist)
{
	writer->Write(dlist->polyType);

	switch (dlist->polyType)
	{
	case 2:
		writer->Write(dlist->x);
		writer->Write(dlist->y);
		writer->Write(dlist->z);
		writer->Write(dlist->unk_06);

		// TODO: DList Stuff
		break;
	default:
		// TODO: DList Stuff
		break;
	}
}
