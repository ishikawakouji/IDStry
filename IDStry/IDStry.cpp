// IDStry.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iostream>

#include <peak/peak.hpp>

using namespace std;

int main()
{
	// initialize library
	peak::Library::Initialize();
	std::cout << "Library successfully initialized!" << std::endl;

	// IDS peak version
	auto peakVersion = peak::Library::Version();
	std::cout << "Library version " << peakVersion.ToString() << std::endl;

	// create a camera manager object
	auto& deviceManager = peak::DeviceManager::Instance();

	try {
		// update to search device
		deviceManager.Update();

		// デバイス取得
		auto devices = deviceManager.Devices();

		// デバイス個数
		auto deviceCount = devices.size();
		cout << deviceCount << " devices found" << endl;

		// デバイス情報
		cout << devices[0]->DisplayName() << endl;
		cout << devices[0]->SerialNumber() << endl;

		// open the first camera
		auto device = deviceManager.Devices().at(0)->OpenDevice(peak::core::DeviceAccessType::Control);

	}
	catch (const std::exception& e)
	{
		std::cout << "EXCEPTION: " << e.what() << std::endl;
		peak::Library::Close();
		return -2;
	}

	// close library before exiting program
	peak::Library::Close();
	return 0;
}

// プログラムの実行: Ctrl + F5 または [デバッグ] > [デバッグなしで開始] メニュー
// プログラムのデバッグ: F5 または [デバッグ] > [デバッグの開始] メニュー

// 作業を開始するためのヒント: 
//    1. ソリューション エクスプローラー ウィンドウを使用してファイルを追加/管理します 
//   2. チーム エクスプローラー ウィンドウを使用してソース管理に接続します
//   3. 出力ウィンドウを使用して、ビルド出力とその他のメッセージを表示します
//   4. エラー一覧ウィンドウを使用してエラーを表示します
//   5. [プロジェクト] > [新しい項目の追加] と移動して新しいコード ファイルを作成するか、[プロジェクト] > [既存の項目の追加] と移動して既存のコード ファイルをプロジェクトに追加します
//   6. 後ほどこのプロジェクトを再び開く場合、[ファイル] > [開く] > [プロジェクト] と移動して .sln ファイルを選択します
