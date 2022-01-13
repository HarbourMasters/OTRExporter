#include "CutsceneExporter.h"
#include <Resource.h>

void OTRExporter_Cutscene::Save(ZResource* res, fs::path outPath, BinaryWriter* writer)
{
	ZCutscene* cs = (ZCutscene*)res;

	writer->Write((uint8_t)Endianess::Little);
	writer->Write((uint32_t)Ship::ResourceType::Cutscene);
	writer->Write((uint32_t)Ship::Version::Deckard);
	writer->Write((uint64_t)0xDEADBEEFDEADBEEF); // id

	//writer->Write((uint32_t)cs->commands.size() + 2 + 2);
	writer->Write((uint32_t)0);

	int currentStream = writer->GetBaseAddress();

	writer->Write(CS_BEGIN_CUTSCENE(cs->numCommands, cs->endFrame));

	for (size_t i = 0; i < cs->commands.size(); i++)
	{
		switch ((CutsceneCommands)cs->commands[i]->commandID)
		{
		case CutsceneCommands::SetCameraPos:
		{
			writer->Write(CS_CMD_CAM_EYE);
			writer->Write(CMD_HH(0x0001, ((CutsceneCommandSetCameraPos*)cs->commands[i])->startFrame));
			writer->Write(CMD_HH(0x0000, ((CutsceneCommandSetCameraPos*)cs->commands[i])->endFrame));

			for (auto& e : ((CutsceneCommandSetCameraPos*)cs->commands[i])->entries)
			{
				writer->Write(CMD_BBH(e->continueFlag, e->cameraRoll, e->nextPointFrame));
				writer->Write(CMD_F(e->viewAngle));
				writer->Write(CMD_HH(e->posX, e->posY));
				writer->Write(CMD_HH(e->posZ, e->unused));
			}
		}
		break;
		case CutsceneCommands::SetCameraFocus:
		{
			writer->Write(CS_CMD_CAM_AT);
			writer->Write(CMD_HH(0x0001, ((CutsceneCommandSetCameraPos*)cs->commands[i])->startFrame));
			writer->Write(CMD_HH(0x0000, ((CutsceneCommandSetCameraPos*)cs->commands[i])->endFrame));

			for (auto& e : ((CutsceneCommandSetCameraPos*)cs->commands[i])->entries)
			{
				writer->Write(CMD_BBH(e->continueFlag, e->cameraRoll, e->nextPointFrame));
				writer->Write(CMD_F(e->viewAngle));
				writer->Write(CMD_HH(e->posX, e->posY));
				writer->Write(CMD_HH(e->posZ, e->unused));
			}
			break;
		}
		case CutsceneCommands::SpecialAction:
		{
			writer->Write(CS_CMD_MISC);
			writer->Write(CMD_W(((CutsceneCommandSpecialAction*)cs->commands[i])->entries.size()));
			for (auto& e : ((CutsceneCommandSpecialAction*)cs->commands[i])->entries) //All in OOT seem to only have 1 entry
			{
				writer->Write(CMD_HH(e->base, e->startFrame));
				writer->Write(CMD_HH(e->endFrame, e->unused0));
				writer->Write(CMD_W(e->unused1));
				writer->Write(CMD_W(e->unused2));
				writer->Write(CMD_W(e->unused3));
				writer->Write(CMD_W(e->unused4));
				writer->Write(CMD_W(e->unused5));
				writer->Write(CMD_W(e->unused6));
				writer->Write(CMD_W(e->unused7));
				writer->Write(CMD_W(e->unused8));
				writer->Write(CMD_W(e->unused9));
				writer->Write(CMD_W(e->unused10));
			}
			break;
		}
		case CutsceneCommands::SetLighting:
		{
			writer->Write(CS_CMD_SET_LIGHTING);
			writer->Write(CMD_W(((CutsceneCommandEnvLighting*)cs->commands[i])->entries.size()));
			for (auto& e : ((CutsceneCommandEnvLighting*)cs->commands[i])->entries)
			{
				writer->Write(CMD_HH(e->setting, e->startFrame));
				writer->Write(CMD_HH(e->endFrame, e->unused0));
				writer->Write(CMD_W(e->unused1));
				writer->Write(CMD_W(e->unused2));
				writer->Write(CMD_W(e->unused3));
				writer->Write(CMD_W(e->unused4));
				writer->Write(CMD_W(e->unused5));
				writer->Write(CMD_W(e->unused6));
				writer->Write(CMD_W(e->unused7));
				writer->Write((uint32_t)0x0);
				writer->Write((uint32_t)0x0);
				writer->Write((uint32_t)0x0);
			}
			break;
		}
		case CutsceneCommands::SetCameraPosLink:
		{
			writer->Write(CS_CMD_CAM_EYE_REL_TO_PLAYER);
			writer->Write(CMD_HH(0x0001, ((CutsceneCommandSetCameraPos*)cs->commands[i])->startFrame));
			writer->Write(CMD_HH(0x0000, ((CutsceneCommandSetCameraPos*)cs->commands[i])->endFrame));

			for (auto& e : ((CutsceneCommandSetCameraPos*)cs->commands[i])->entries)
			{
				writer->Write(CMD_BBH(e->continueFlag, e->cameraRoll, e->nextPointFrame));
				writer->Write(CMD_F(e->viewAngle));
				writer->Write(CMD_HH(e->posX, e->posY));
				writer->Write(CMD_HH(e->posZ, e->unused));
			}
			break;
		}
		case CutsceneCommands::SetCameraFocusLink:
		{
			writer->Write(CS_CMD_CAM_AT_REL_TO_PLAYER);
			writer->Write(CMD_HH(0x0001, ((CutsceneCommandSetCameraPos*)cs->commands[i])->startFrame));
			writer->Write(CMD_HH(0x0000, ((CutsceneCommandSetCameraPos*)cs->commands[i])->endFrame));

			for (auto& e : ((CutsceneCommandSetCameraPos*)cs->commands[i])->entries)
			{
				writer->Write(CMD_BBH(e->continueFlag, e->cameraRoll, e->nextPointFrame));
				writer->Write(CMD_F(e->viewAngle));
				writer->Write(CMD_HH(e->posX, e->posY));
				writer->Write(CMD_HH(e->posZ, e->unused));
			}
			break;
		}

		case CutsceneCommands::Cmd07: // Not used in OOT
			break;
		case CutsceneCommands::Cmd08: // Not used in OOT
			break;

		case CutsceneCommands::Cmd09:
		{
			writer->Write(CS_CMD_09);
			writer->Write(CMD_W(((CutsceneCommandUnknown9*)cs->commands[i])->entries.size()));

			for (auto& e : ((CutsceneCommandUnknown9*)cs->commands[i])->entries)
			{
				writer->Write(CMD_HH(e->base, e->startFrame));
				writer->Write(CMD_HBB(e->endFrame, e->unk2, e->unk3));
				writer->Write(CMD_BBH(e->unk4, e->unused0, e->unused1));
			}
			break;
		}

		case CutsceneCommands::Textbox:
		{
			writer->Write(CS_CMD_TEXTBOX);
			writer->Write(CMD_W(((CutsceneCommandTextbox*)cs->commands[i])->entries.size()));

			for (auto& e : ((CutsceneCommandTextbox*)cs->commands[i])->entries)
			{
				if (e->base == 0xFFFF) // CS_TEXT_NONE
				{
					writer->Write(CMD_HH(0xFFFF, e->startFrame));
					writer->Write(CMD_HH(e->endFrame, 0xFFFF));
					writer->Write(CMD_HH(0xFFFF, 0xFFFF));
				}
				else // CS_TEXT_DISPLAY_TEXTBOX
				{
					writer->Write(CMD_HH(e->base, e->startFrame));
					writer->Write(CMD_HH(e->endFrame, e->type));
					writer->Write(CMD_HH(e->textID1, e->textID2));
				}
			}
			break;
		}
		case CutsceneCommands::SetActorAction0:
		case CutsceneCommands::SetActorAction1:
		case CutsceneCommands::SetActorAction2:
		case CutsceneCommands::SetActorAction3:
		case CutsceneCommands::SetActorAction4:
		case CutsceneCommands::SetActorAction5:
		case CutsceneCommands::SetActorAction6:
		case CutsceneCommands::SetActorAction7:
		case CutsceneCommands::SetActorAction8:
		case CutsceneCommands::SetActorAction9:
		case CutsceneCommands::SetActorAction10:
		{
			// OTRTODO this one is weird
			break;
		}
		case CutsceneCommands::SetSceneTransFX:
		{
			CutsceneCommandSceneTransFX* cmdTFX = (CutsceneCommandSceneTransFX*)cs->commands[i];

			writer->Write(CS_CMD_SCENE_TRANS_FX);
			writer->Write((uint32_t)1);
			writer->Write(CMD_HH((((CutsceneCommandSceneTransFX*)cs->commands[i])->base), ((CutsceneCommandSceneTransFX*)cs->commands[i])->startFrame));
			writer->Write(CMD_HH((((CutsceneCommandSceneTransFX*)cs->commands[i])->endFrame), ((CutsceneCommandSceneTransFX*)cs->commands[i])->endFrame));
			break;
		}
		case CutsceneCommands::Nop: //Not used in OOT
			break;
		case CutsceneCommands::PlayBGM:
		{
			writer->Write(CS_CMD_PLAYBGM);
			writer->Write(CMD_W(((CutsceneCommandPlayBGM*)cs->commands[i])->entries.size()));

			for (auto& e : ((CutsceneCommandPlayBGM*)cs->commands[i])->entries)
			{
				writer->Write(CMD_HH(e->sequence, e->startFrame));
				writer->Write(CMD_HH(e->endFrame, e->unknown0));
				writer->Write(CMD_W(e->unknown2));
				writer->Write(CMD_W(e->unknown3));
				writer->Write(CMD_W(e->unknown4));
				writer->Write(CMD_W(e->unknown5));
				writer->Write(CMD_W(e->unknown6));
				writer->Write(CMD_W(e->unknown7));
				writer->Write((uint32_t)0);
				writer->Write((uint32_t)0);
				writer->Write((uint32_t)0);
			}
			break;
		}
		case CutsceneCommands::StopBGM:
		{
			writer->Write(CS_CMD_STOPBGM);
			writer->Write(CMD_W(((CutsceneCommandStopBGM*)cs->commands[i])->entries.size()));

			for (auto& e : ((CutsceneCommandStopBGM*)cs->commands[i])->entries)
			{
				writer->Write(CMD_HH(e->sequence, e->startFrame));
				writer->Write(CMD_HH(e->endFrame, e->unknown0));
				writer->Write(CMD_W(e->unknown2));
				writer->Write(CMD_W(e->unknown3));
				writer->Write(CMD_W(e->unknown4));
				writer->Write(CMD_W(e->unknown5));
				writer->Write(CMD_W(e->unknown6));
				writer->Write(CMD_W(e->unknown7));
				writer->Write((uint32_t)0);
				writer->Write((uint32_t)0);
				writer->Write((uint32_t)0);
			}
			break;
		}
		case CutsceneCommands::FadeBGM:
		{
			writer->Write(CS_CMD_FADEBGM);
			writer->Write(CMD_W(((CutsceneCommandFadeBGM*)cs->commands[i])->entries.size()));

			for (auto& e : ((CutsceneCommandFadeBGM*)cs->commands[i])->entries)
			{
				writer->Write(CMD_HH(e->base, e->startFrame));
				writer->Write(CMD_HH(e->endFrame, e->unknown0));
				writer->Write(CMD_W(e->unknown2));
				writer->Write(CMD_W(e->unknown3));
				writer->Write(CMD_W(e->unknown4));
				writer->Write(CMD_W(e->unknown5));
				writer->Write(CMD_W(e->unknown6));
				writer->Write(CMD_W(e->unknown7));
				writer->Write((uint32_t)0);
				writer->Write((uint32_t)0);
				writer->Write((uint32_t)0);
			}
			break;
		}
		case CutsceneCommands::SetTime:
		{
			writer->Write(CS_CMD_SETTIME);
			writer->Write(CMD_W(((CutsceneCommandDayTime*)cs->commands[i])->entries.size()));

			for (auto& e : ((CutsceneCommandDayTime*)cs->commands[i])->entries)
			{
				writer->Write(CMD_HH(e->base, e->startFrame));
				writer->Write(CMD_HBB(e->endFrame, e->hour, e->minute));
				writer->Write(CMD_W(e->unused));
			}
			break;
		}
		case CutsceneCommands::Terminator:
		{
			writer->Write(CS_CMD_TERMINATOR);
			writer->Write((uint32_t)1);
			writer->Write(CMD_HH(((CutsceneCommandTerminator*)cs->commands[i])->base, ((CutsceneCommandTerminator*)cs->commands[i])->startFrame));
			writer->Write(CMD_HH(((CutsceneCommandTerminator*)cs->commands[i])->endFrame, ((CutsceneCommandTerminator*)cs->commands[i])->endFrame));
			break;
		}
		}
	}

	//CS_END
	writer->Write(0xFFFFFFFF);
	writer->Write((uint32_t)0);

	int endStream = writer->GetBaseAddress();
	writer->Seek(currentStream - 4, SeekOffsetType::Start);
	writer->Write((uint32_t)((endStream - currentStream) / 4));
	writer->Seek(endStream, SeekOffsetType::Start);
}