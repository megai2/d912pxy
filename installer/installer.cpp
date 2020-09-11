#include <iostream>
#include <string>
#include <windows.h>
#include <sys/stat.h>

int install_next_to_d912pxy_folder = 0;

int IsFileExist(const char *name)
{
	struct stat   buffer;
	return (stat(name, &buffer) == 0);
}

bool CheckLocation()
{
	return IsFileExist("../Gw2-64.exe");
}

int read_user_integer(int defv)
{
	int ret = defv;
	std::string str;

	getline(std::cin, str);

	std::cout << "\n";

	if (str.length() == 0)
		ret = defv;
	else {
		try {
			ret = std::stoi(str);
		}
		catch (const std::invalid_argument &) {
			return -1;
		}
	}

	return ret;
}

bool ReadUserYN(bool defaultY)
{
	if (defaultY)
	{
		std::cout << "Yes/No \n[default: Yes]: ";
	} else
		std::cout << "Yes/No \n[default: No]: ";

	std::string str;

	getline(std::cin, str);

	std::cout << "\n";

	if ((str[0] == 'y') || (str[0] == 'Y'))
	{
		return 1;
	}
	else {
		if (str.length() == 0)
			return defaultY;
		else
			return 0;
	}

}

#include "../thirdparty/cpu_arch_test.inc"

int action_install()
{
	std::string installPath = "..\\bin64\\";
	std::string installSource = "release\\";

	cpu_arch arch = GetCPUArch();

	if (arch.AVX2)
		installSource = "release_avx2\\";
	else if (arch.AVX)
		installSource = "release_avx\\";
	else if (!arch.SSE)
	{
		std::cout << "Your CPU need SSE support in order to use this tool\n";
		system("pause");
		return -1;
	}

	std::string installFile = "d3d9.dll";

	if (!install_next_to_d912pxy_folder)
	{
		std::cout << "Install in bin64? ";

		if (!ReadUserYN(1))
		{
			std::cout << "Installing in game root folder. \n";
			std::cout << "Please note that this typically is not needed! \n";
			std::cout << "You will probably have to troubleshoot this configuration yourself. \n";
			std::cout << "Report on this issue here: https://github.com/megai2/d912pxy/issues/64 \n\n";
			installPath = "..\\";
		}
	}
	else {
		installPath = "..\\";

		std::cout << "Use special config? \n"
			<< "1. No \n"
			<< "2. Default config for BnS \n"
			<< "3. Exit \n";

		std::cout << "\n[default: 2, config for BnS ]: ";

		int mode = read_user_integer(2);

		switch (mode)
		{
		case 1:
			break;
		case 2:
			if (system("copy config.ini backup_config.ini") != 0)
				std::cout << "failed to backup config \n";

			if (system("copy bns_config.ini config.ini") != 0)
				std::cout << "failed to copy config \n";

			break;
		default:
			std::cout << "exiting";
			system("pause");
			return 0;

			break;
		}
	}

	std::cout << "Use standard release? ";

	if (!ReadUserYN(1))
	{
		std::cout << "Select configuration: \n\n";
		std::cout << "1. Release_pp  - performance data collection \n";
		std::cout << "2. Release_d   - in-depth debug logging \n";
		std::cout << "3. Debug       - debug build\n";

		std::cout << "\n[default: Release_pp]: ";

		int mode = read_user_integer(1);

		switch (mode)
		{
		case 1:
			installSource = "release_pp\\";
			break;
		case 2:
			installSource = "release_d\\";
			break;
		case 3:
			installSource = "debug\\";
			break;
		default:
			std::cout << "Incorrect parameter; exiting.\n";
			system("pause");
			return -1;
		}
	}

	std::cout << "Install as d3d9.dll? \n";
	std::cout << "(Choose \"No\" if you use mods that modify d3d9.dll) ";

	int mode = 0;

	if (!ReadUserYN(1))
	{
		std::cout << "Select name variant: \n\n";
		std::cout << "1. d3d9_chainload.dll \n";
		std::cout << "2. d3d9_mchain.dll \n";
		std::cout << "3. d912pxy.dll \n";//megai2: who knows?

		std::cout << "\n[default: d3d9_chainload.dll]: ";

		mode = read_user_integer(1);

		switch (mode)
		{
		case 1:
			installFile = "d3d9_chainload.dll";
			break;
		case 2:
			installFile = "d3d9_mchain.dll";
			break;
		case 3:
			installFile = "d912pxy.dll";
			break;
		default:
			std::cout << "Incorrect parameter; exiting.\n";
			system("pause");
			return -1;
		}
	}

	std::string fCpy = "";
	std::string fBkp = "";
	std::string fIfn = installPath + installFile;

	fCpy += "copy dll\\";
	fCpy += installSource;
	fCpy += "d3d9.dll";
	fCpy += " ";
	fCpy += fIfn;

	fBkp += "copy ";
	fBkp += fIfn;
	fBkp += " ";
	fBkp += installPath;
	fBkp += "d912pxy_installer_backup._ll";

	if (IsFileExist(fIfn.c_str()))
	{
		std::cout << "Backing up old file: " << fBkp << " \n";
		if (system(fBkp.c_str()) != 0)
		{
			std::cout << "Backup failed! Exiting.\n";
			system("pause");
			return -1;
		}
	}

	std::cout << "\nInstalling... \n";

	if (system(fCpy.c_str()) != 0)
	{
		std::cout << "Installation to " << fIfn << " from dll\\" << installSource << "d3d9.dll failed! Exiting.\n";
		system("pause");
		return -1;
	}


	FILE* f = NULL;
	if (!fopen_s(&f, "installed.flag", "wb"))
	{
		fwrite(&mode, sizeof(int), 1, f);

		mode = installPath.length() < 6;
		fwrite(&mode, sizeof(int), 1, f);

		fflush(f);
		fclose(f);

		std::cout << "Installation complete. \n";
	}
	else {
		std::cout << "Failed to write installation info. You should remove the file '" << fIfn << "' by hand if you want to remove this program. \n";
	}

	system("pause");

	return 0;
}

int action_remove()
{
	FILE* f = NULL;

	if (fopen_s(&f, "installed.flag", "rb"))
	{
		std::cout << "No installation found. Remove the installed d3d9.dll (or other) file and this folder by hand to uninstall manually. \n";

		system("pause");
		return 0;
	}

	int installData[2] = { 0, 0 };

	fread(&installData[0], sizeof(int), 2, f);
	fclose(f);


	std::string installPath = "..\\bin64\\";
	std::string installFile = "d3d9.dll";

	if (installData[1])
	{
		installPath = "..\\";
	}


	switch (installData[0])
	{
	case 0:
		break;
	case 1:
		installFile = "d3d9_chainload.dll";
		break;
	case 2:
		installFile = "d3d9_mchain.dll";
		break;
	case 3:
		installFile = "d912pxy.dll";
		break;
	default:
		std::cout << "Incorrect installation data. Remove the installed d3d9.dll (or other) file and this folder by hand to uninstall manually. Exiting.\n";
		system("pause");
		return -1;
	}

	std::string fIfn = installPath + installFile;
	std::string fBkp = installPath + "d912pxy_installer_backup._ll";

	if (IsFileExist(fBkp.c_str()))
	{
		std::cout << "Found backup. Restoring: \n";

		std::string fRst = "";

		fRst += "copy ";
		fRst += fBkp;
		fRst += " ";
		fRst += fIfn;

		if (system(fRst.c_str()) != 0)
			std::cout << "Failed to restore backup! \n";
		else
			std::cout << "Backup restored \n";
	} else {
		std::string fDel = "del /Q ";
		fDel += fIfn;

		if (system(fDel.c_str()) != 0)
			std::cout << "Failed to delete target installation file " << fIfn << " \n";
		else
			std::cout << "Installation removed \n";
	}

	if (system("del /Q installed.flag") != 0)
	{
		std::cout << "Can't clean up install.flag file \n";
	}

	system("pause");

	return 0;
}

int action_clear_shader_cache()
{
	std::cout << "Deleting latest.pck in pck and pck_bns \n";

	system("del /Q pck_bns\\latest.pck");
	system("del /Q pck\\latest.pck");

	std::cout << "Perform additional hlsl sources cleaning? ";

	if (ReadUserYN(0))
	{
		system("del /Q shaders\\hlsl\\*");
	}

	std::cout << "Finished \n";

	system("pause");

	return 0;
}

int action_exit()
{
	std::cout << "Exiting \n";
	system("pause");

	return 0;
}

int main()
{
	std::cout << "d912pxy installer\n\n";

	std::cout << "WARNING: USE OF THIS SOFTWARE IS ENTIRELY AT YOUR OWN RISK!\n";

	if (!ReadUserYN(1))
	{
		system("pause");
		return -1;
	}

	std::cout << "WARNING: You should disable any overlay software installed \n";
	std::cout << "unless that software is listed as compatible. \n";

	if (!ReadUserYN(1))
	{
		system("pause");
		return -1;
	}

	std::cout << "If you use ArcDPS, GW2Radial, GW2Hook, GW2Reshade, \n";
	std::cout << "or any other mod that uses \"d3d9.dll\" file, you SHOULD read this first! \n";
	std::cout << "https://github.com/megai2/d912pxy/issues/38 \n";

	if (!ReadUserYN(1))
	{
		system("pause");
		return -1;
	}

	std::cout << "Use default answers if you don't understand the prompts! \n";

	if (!ReadUserYN(1))
	{
		system("pause");
		return -1;
	}

	if (!CheckLocation())
	{
		std::cout << "\n=============================================\n";
		std::cout << "No Gw2-64.exe found, install next to d912pxy folder? \n\n";

		if (!ReadUserYN(1))
		{

			std::cout << "You should put the d912pxy folder into the game's root directory. \n";
			std::cout << "Example: \n\n";

			std::cout << "X:/games/GuildWars2/ \n";
			std::cout << "|-Gw2.dat \n";
			std::cout << "|-Gw2-64.exe \n";
			std::cout << "|-d912pxy \n";
			std::cout << "	|-install.exe \n\n";

			system("pause");

			return -1;
		}
		else {
			install_next_to_d912pxy_folder = 1;
		}
	}

	std::cout << "Choose an action: \n"
		<< "1. Install \n"
		<< "2. Remove \n"
		<< "3. Clear shader cache \n"
		<< "4. Exit \n";

	std::cout << "\n[default: 1, Install]: ";

	int mode = read_user_integer(1);

	switch (mode)
	{
		case 1:
			return action_install();
		case 2:
			return action_remove();
		case 3:
			return action_clear_shader_cache();
		case 4:
			return action_exit();
		default:
			std::cout << "Incorrect parameter; exiting.\n";
			return -1;
	}

}
