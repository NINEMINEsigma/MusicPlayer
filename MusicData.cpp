#include "MusicData.h"


/****************************************************************************************************************
 *  class MusicData																								*
 ****************************************************************************************************************/

/****************************************************************
 * MusicData private method										*
 ****************************************************************/


FILE* STD_COUT_STREAM = nullptr;								// 指向标准输出流的文件指针


// 打开并播放音乐
bool MusicData::openMusic(size_t num)
{
	if (musicPathName.empty() || num > musicPathName.size() - 1)
	{
		console.LogError("参数不符合要求，请重试！");

		status = 0;
		return false;
	}

	nowMusicName = musicName.at(num);								// 设置正在操作的音乐名
	number = num;													// 设置正在操作的音乐名编号

	if (musicMci.open(musicPathName.at(num).c_str()))
	{
		console.LogMessage(std::string("音乐 ") + to_string(nowMusicName) + " 打开成功");

		playMusic();
		musicMci.setVolume(volume);
		return true;
	}
	else
	{
		console.LogError(std::string("音乐 ") + to_string(nowMusicName) + " 打开失败");
		return false;
	}
}

// 播放音乐
void MusicData::playMusic()
{
	if (musicMci.play())
	{
		console.LogMessage(std::string("音乐 ") + to_string(nowMusicName) + " 播放");
		status = 1;
	}
	else
	{
		console.LogError(std::string("音乐 ") + to_string(nowMusicName) + " 播放失败");

		status = 0;
	}
	if (std::cout.fail())
	{
		std::cout.setstate(std::ios::goodbit);
		std::cout.rdbuf();
	}
	if (std::wcout.fail())
	{
		std::wcout.setstate(std::ios::goodbit);
		std::wcout.rdbuf();
	}
	if (ld::clog->fail())
	{
		ld::clog->setstate(std::ios::goodbit);
		ld::clog->rdbuf();
	}
}

// 暂停播放
void MusicData::pauseMusic()
{
	if (status)
	{
		if (musicMci.pause())
		{
			console.LogMessage(std::string("音乐 ") + to_string(nowMusicName) + " 暂停");
			status = 2;
		}
		else
		{
			console.LogError(std::string("音乐 ") + to_string(nowMusicName) + " 暂停失败");
		}
	}
}

// 停止播放
void MusicData::stopMusic()
{
	if (musicMci.stop())
	{
		console.LogMessage(std::string("音乐 ") + to_string(nowMusicName) + " 停止");
		status = 0;
	}
	else
	{
		console.LogError(std::string("音乐 ") + to_string(nowMusicName) + " 停止失败");
	}
}

// 关闭音乐
void MusicData::closeMusic()
{
	if (musicMci.close())
	{
		console.LogMessage(std::string("音乐 ") + to_string(nowMusicName) + " 已关闭");
		status = 0;
	}
	else
	{
		console.LogError(std::string("音乐 ") + to_string(nowMusicName) + " 关闭失败");
	}
}

// 设置音乐音量
// 参数允许范围，0 - 1000
void MusicData::setMusicVolume(size_t vol)
{
	if (status)
	{
		if (musicMci.setVolume(vol))
			volume = vol;
		else
		{
			console.LogError("音量设置失败");
		}
	}
	else
	{
		volume = vol;
	}
}

// 设置播放位置
// 参数单位，ms
bool MusicData::setMusicStartTime(size_t start_time)
{
	if (status)
	{
		if (musicMci.setStartTime(start_time))
		{
			return true;
		}
		else
		{
			console.LogError("设置播放位置失败");
			return false;
		}
	}

	return false;
}

// 获取音乐当前播放时间，单位 s
int MusicData::getMusicCurrentTime()
{
	if (status)
	{
		DWORD playTime = 0;
		if (!musicMci.getCurrentTime(playTime))
		{
			console.LogError("获取播放时长失败");

			return 0;
		}
		return playTime / 1000;
	}

	return 0;
}

// 获取音乐总播放时间，单位 s
int MusicData::getMusicTotalTime()
{
	if (status)
	{
		DWORD totalTime = 0;
		if (!musicMci.getTotalTime(totalTime))
		{
			console.LogError("获取总时长失败");

			return 0;
		}
		return totalTime / 1000;
	}

	return 0;
}

// 播放上一曲
void MusicData::prevMusic()
{
	if (status)													// 是否需要关闭音乐
	{
		closeMusic();
	}

	if (mode == 0 || mode == 1)									// 若不是随机播放，上一曲
	{
		if (openMusic(number == 0 ? musicName.size() - 1 : number - 1) == false)
			prevMusic();
	}
	else														// 是随机播放，随机一曲
	{
		while (openMusic(rand() % musicName.size()) == false);
	}
}

// 播放下一曲
void MusicData::nextMusic()
{
	if (status)
	{
		closeMusic();
	}

	if (mode == 0 || mode == 1)
	{
		if (openMusic(number + 1 > musicName.size() - 1 ? 0 : number + 1) == false)
			nextMusic();
	}
	else
	{
		while (openMusic(rand() % musicName.size()) == false);
	}
}


// 从文件 filePath.ini 中读取搜索路径
void MusicData::getFilePath()
{
	FILE* fp = nullptr;
	auto err = _wfopen_s(&fp, L"filePath.ini", L"r");
	if (err != 0)
	{
		console.LogWarning("文件 filePath.ini 打开失败");
		console.LogWarning("程序将新建文件 filePath.ini");

		err = _wfopen_s(&fp, L"filePath.ini", L"w, ccs=UTF-16LE");
		if (err == 0)
		{
			fputws(L"; 请在“path=”后写入合适的音乐搜索路径\n", fp);
			fputws(L"; 注意，请在修改后确保文件编码为 UTF-16LE，若不是，请将文件另存为 UTF-16LE 编码并保存\n", fp);
			// 写入配置文件注释
			fclose(fp);										// 关闭文件

			filePath = L"C:/";								// 设置默认为C盘
			wFilePath();									// 写入配置信息
		}
	}
	else
	{
		rFilePath();										// 读取配置信息
		fclose(fp);											// 关闭文件
	}
}

// 获取特定格式的文件名    
void MusicData::findMusicName(const wstring& path)
{
	if (std::cout.fail())
	{
		std::cout.setstate(std::ios::goodbit);
		std::cout.rdbuf();
	}
	if (std::wcout.fail())
	{
		std::wcout.setstate(std::ios::goodbit);
		std::wcout.rdbuf();
	}
	if (ld::clog->fail())
	{
		ld::clog->setstate(std::ios::goodbit);
		ld::clog->rdbuf();
	}
	console.LogWarning(path + L" (目前正在搜索)...");
	WIN32_FIND_DATA fileinfo;														// 文件信息
	HANDLE hFind;
	for (auto& musicFormat : musicFormats)
	{
		auto hFind = FindFirstFile((path + L"\\*." + musicFormat).c_str(), &fileinfo);	// 查找当前文件夹是否存在指定格式文件
		if (hFind != INVALID_HANDLE_VALUE)												// 找到指定格式文件
		{
			do
			{
				if (std::cout.fail())
				{
					std::cout.setstate(std::ios::goodbit);
					std::cout.rdbuf();
				}
				if (std::wcout.fail())
				{
					std::wcout.setstate(std::ios::goodbit);
					std::wcout.rdbuf();
				}
				if (ld::clog->fail())
				{
					ld::clog->setstate(std::ios::goodbit);
					ld::clog->rdbuf();
				}
				wstring str = fileinfo.cFileName;
				console.LogMessage(std::wstring(L"找到了: ") + str);
				musicPathName.push_back(path + L"\\" + str);							// 写入音乐全路径名
				musicName.push_back(str.substr(0, str.size() - musicFormat.size() - 1));// 写入音乐名
			} while (FindNextFile(hFind, &fileinfo));
		}
		FindClose(hFind);
	}

	hFind = FindFirstFile((path + L"\\*").c_str(), &fileinfo);						// 查找子文件夹，查找所有文件
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if ((fileinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))				// 判断是否为文件夹
			{
				if (fileinfo.cFileName[0] != L'.')									// 过滤文件夹 "." 和 ".."
				{
					findMusicName(path + L"\\" + fileinfo.cFileName);				// 递归遍历文件夹
				}
			}
		} while (FindNextFile(hFind, &fileinfo));
	}
	FindClose(hFind);
}

// 将 filePath 写入文件 filePath.ini
void MusicData::wFilePath()
{
	// 在 .\\filePath.ini 中写入 节 [filePath] 键=值 path=filePath
	WritePrivateProfileString(L"filePath", L"path", filePath.c_str(), L".\\filePath.ini");
}

// 将文件 filePath.ini 读取到 filePath
void MusicData::rFilePath()
{
	// 读取对应节，键的值
	array<wchar_t, 256> wptr{ L'\0' };
	const DWORD d = GetPrivateProfileString(L"filePath", L"path", L".",
		&wptr.at(0), 256, L".\\filePath.ini");

	filePath = &wptr.at(0);
}

// 将 musicPathName 写入文件 music.mn 中
void MusicData::wFileMusic(FILE* fp)
{
	if (!musicPathName.empty())
	{
		const int len = static_cast<int>(musicPathName.size());
		fputws(musicPathName.at(0).c_str(), fp);

		for (int i = 1; i < len; ++i)
		{
			fputws(L"\n", fp);
			fputws(musicPathName.at(i).c_str(), fp);
		}
		console.LogMessage("文件 music.mn 写入完毕");
	}
	else
	{
		console.LogWarning("musicPathName 为空");
	}
}

// 从 music.mn 读取到 musicPathName 和 musicName 中
void MusicData::rFileMusic(FILE* fp)
{
	wstring s;
	array<wchar_t, 256> wMusic{ L'\0' };

	while (!feof(fp))
	{
		fgetws(&wMusic.at(0), 256, fp);
		s = &wMusic.at(0);
		if (s.empty())
		{
			continue;															// 若为空，跳过
		}
		if (s.back() == L'\n')
		{
			s.pop_back();														// 去掉末尾的换行符
			if (s.empty())
			{
				continue;
			}
		}
		musicPathName.push_back(s);												// 写入音乐全路径名
		const auto pos = s.rfind(L"\\");
		std::wcout << ld::ConsoleColor::Green << s << L"->读取" << std::endl;
		if (std::cout.fail())
		{
			std::cout.setstate(std::ios::goodbit);
			std::cout.rdbuf();
		}
		if (std::wcout.fail())
		{
			std::wcout.setstate(std::ios::goodbit);
			std::wcout.rdbuf();
		}
		if (ld::clog->fail())
		{
			ld::clog->setstate(std::ios::goodbit);
			ld::clog->rdbuf();
		}
		musicName.push_back(s);													// 写入音乐名
	}
	console.LogMessage("文件 musci.mn 读取完毕");
	if (musicPathName.empty())
	{
		console.LogWarning("musicPathName 为空");
	}
}


/****************************************************************
 *  MusicData public method										*
 ****************************************************************/

 // 默认构造函数
MusicData::MusicData()
{
	srand(time(nullptr) & 0xffffffff);

	AllocConsole();														// 添加控制台
	freopen_s(&STD_COUT_STREAM, "CON", "w", stdout);					// 重定向标准输出流
	std::wcout.imbue(std::locale("", LC_CTYPE));


	FILE* fp = nullptr;													// 文件指针
	auto err = _wfopen_s(&fp, L"music.mn", L"r, ccs=UTF-16LE");			// 以 只读，UTF-16LE 编码格式打开文件
	if (err == 0)														// 打开成功
	{
		rFileMusic(fp);													// 若文件存在，直接读取其内容
		fclose(fp);														// 关闭文件
	}
	else
	{
		console.LogWarning ("文件 music.mn 打开失败");
		console.LogWarning("程序将新建文件 music.mn，并进行音乐搜索");

		err = _wfopen_s(&fp, L"music.mn", L"w, ccs=UTF-16LE");			// 创建以 UTF-16LE 编码的文件
		if (err == 0)													// 打开成功，创建成功
		{
			console.LogMessage("文件 music.mn 创建成功");

			getFilePath();												// 获取音乐的搜索路径

			console.LogMessage("音乐搜索路径为：");
			console.LogMessage(to_string(filePath));
			console.LogMessage("音乐搜索中...");

			findMusicName(filePath);

			console.LogMessage("音乐搜索完毕");

			wFileMusic(fp);												// 写入
			fclose(fp);													// 关闭它
		}
	}
}

// 析构函数
MusicData::~MusicData()
{
	closeMusic();

#ifdef _DEBUG
	fclose(STD_COUT_STREAM);
#endif // _DEBUG
}