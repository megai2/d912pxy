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

void WaitNewLine()
{
	std::cout << "Press enter to continue";

	std::string str;

	getline(std::cin, str);
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

	//block AVX builds for now, as they don't have any actuall usage of AVX,
	//simply clocking down CPU due to AVX offset
	//if (arch.AVX2)
	//	installSource = "release_avx2\\";
	//else if (arch.AVX)
	//	installSource = "release_avx\\";
	//else 
	if (!arch.SSE)
	{
		std::cout << "Your CPU need SSE support in order to use this tool\n";
		WaitNewLine();
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
			if (!CopyFile(L"config.ini", L"backup_config.ini", false))
				std::cout << "failed to backup config \n";

			if (!CopyFile(L"bns_config.ini", L"config.ini", false))
				std::cout << "failed to copy config \n";

			break;
		default:
			std::cout << "exiting";
			WaitNewLine();
			return 0;

			break;
		}
	}

	std::cout << "Use standard release? ";

	if (!ReadUserYN(1))
	{
		std::cout << "Select configuration: \n\n";
		std::cout << "1. Debug        - debug build\n";
		std::cout << "2. 32-bit       - 32bit build\n";
		std::cout << "3. Debug 32-bit - 32bit debug build\n";

		std::cout << "\n[default: debug]: ";

		int mode = read_user_integer(1);

		switch (mode)
		{
		case 1:
			installSource = "debug\\";
			break;
		case 2:
			installSource = "release_x86\\";
			break;
		case 3:
			installSource = "debug_x86\\";
			break;
		default:
			std::cout << "Incorrect parameter; exiting.\n";
			WaitNewLine();
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
			WaitNewLine();
			return -1;
		}
	}

	std::string fInstallSource = "dll\\" + installSource + "d3d9.dll";
	std::string fTargetFile = installPath + installFile;
	std::string fBackupFile = installPath + "d912pxy_installer_backup._ll";

	if (IsFileExist(fTargetFile.c_str()))
	{
		std::cout << "Backing up old file: " << fTargetFile.c_str() << " \n";
		if (!CopyFileA(fTargetFile.c_str(), fBackupFile.c_str(), false))
		{
			std::cout << "Backup failed! Exiting.\n";
			WaitNewLine();
			return -1;
		}
	}

	std::cout << "\nInstalling... \n";

	if (!CopyFileA(fInstallSource.c_str(), fTargetFile.c_str(), false))
	{
		std::cout << "Installation to " << fTargetFile.c_str() << " from dll\\" << installSource << "d3d9.dll failed! Exiting.\n";
		WaitNewLine();
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
		std::cout << "Failed to write installation info. You should remove the file '" << fTargetFile.c_str() << "' by hand if you want to remove this program. \n";
	}

	WaitNewLine();

	return 0;
}

int action_remove()
{
	FILE* f = NULL;

	if (fopen_s(&f, "installed.flag", "rb"))
	{
		std::cout << "No installation found. Remove the installed d3d9.dll (or other) file and this folder by hand to uninstall manually. \n";

		WaitNewLine();
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
		WaitNewLine();
		return -1;
	}

	std::string fIfn = installPath + installFile;
	std::string fBkp = installPath + "d912pxy_installer_backup._ll";

	if (IsFileExist(fBkp.c_str()))
	{
		std::cout << "Found backup. Restoring: \n";

		if (!CopyFileA(fBkp.c_str(), fIfn.c_str(), false))
			std::cout << "Failed to restore backup! \n";
		else
			std::cout << "Backup restored \n";
	} else {
		if (!DeleteFileA(fIfn.c_str()))
			std::cout << "Failed to delete target installation file " << fIfn << " \n";
		else
			std::cout << "Installation removed \n";
	}

	if (!DeleteFileA("installed.flag"))
		std::cout << "Can't clean up install.flag file \n";

	WaitNewLine();

	return 0;
}

int action_clear_shader_cache()
{
	std::cout << "Deleting latest.pck in pck and pck_bns \n";

	DeleteFileA("pck_bns\\latest.pck");
	DeleteFileA("pck\\latest.pck");

	std::cout << "Finished \n";

	WaitNewLine();

	return 0;
}

int action_exit()
{
	std::cout << "Exiting \n";
	WaitNewLine();

	return 0;
}

int main()
{
	std::cout << "d912pxy installer\n\n";

	std::cout << "WARNING: USE OF THIS SOFTWARE IS ENTIRELY AT YOUR OWN RISK!\n";

	if (!ReadUserYN(1))
	{
		WaitNewLine();
		return -1;
	}

	std::cout << "WARNING: You should disable any overlay software installed \n";
	std::cout << "unless that software is listed as compatible. \n";

	if (!ReadUserYN(1))
	{
		WaitNewLine();
		return -1;
	}

	std::cout << "If you use ArcDPS, GW2Radial, GW2Hook, GW2Reshade, \n";
	std::cout << "or any other mod that uses \"d3d9.dll\" file, you SHOULD read this first! \n";
	std::cout << "https://github.com/megai2/d912pxy/issues/38 \n";

	if (!ReadUserYN(1))
	{
		WaitNewLine();
		return -1;
	}

	std::cout << "Use default answers if you don't understand the prompts! \n";

	if (!ReadUserYN(1))
	{
		WaitNewLine();
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

			WaitNewLine();

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
