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

	// device val
	shared_ptr < peak::core::Device> device = NULL;

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
		device = deviceManager.Devices().at(0)->OpenDevice(peak::core::DeviceAccessType::Control);

		// Get the RemoteDevice NodeMap
		auto nodeMapRemoteDevice = device->RemoteDevice()->NodeMaps().at(0);

		// ExposureTime
		nodeMapRemoteDevice->FindNode<peak::core::nodes::FloatNode>("ExposureTime")->SetValue(1000.0);
		cout << "ExposureTime " << nodeMapRemoteDevice->FindNode<peak::core::nodes::FloatNode>("ExposureTime")->Value() << endl;

		// software trigger
		// Before accessing TriggerSource, make sure TriggerSelector is set correctly
		// Set Trigger Selector "ExposureStart"

		nodeMapRemoteDevice->FindNode<peak::core::nodes::EnumerationNode>("TriggerSelector")->SetCurrentEntry("ExposureStart");

		// Determine the current entry of TriggerSource
		std::string value = nodeMapRemoteDevice->FindNode<peak::core::nodes::EnumerationNode>("TriggerSource")->CurrentEntry()->SymbolicValue();
		cout << "current Source: " << value << endl;

		// Get a list of all available entries of TriggerSource
		auto allEntries = nodeMapRemoteDevice->FindNode<peak::core::nodes::EnumerationNode>("TriggerSource")->Entries();

		cout << "available entries:" << endl;
		std::vector<std::shared_ptr<peak::core::nodes::EnumerationEntryNode>> availableEntries;
		for (const auto& entry : allEntries)
		{
			if ((entry->AccessStatus() != peak::core::nodes::NodeAccessStatus::NotAvailable)
				&& (entry->AccessStatus() != peak::core::nodes::NodeAccessStatus::NotImplemented))
			{
				availableEntries.emplace_back(entry);
				cout << "\t"  << entry->SymbolicValue() << endl;
			}
		}

		// Set TriggerSource to "Software"
		nodeMapRemoteDevice->FindNode<peak::core::nodes::EnumerationNode>("TriggerSource")->SetCurrentEntry("Software");

	}
	catch (const peak::core::OutOfRangeException& e)
	{
		// The value is out of range, e.g. higher than Maximum or lower than Minimum
		std::cout << "Range EXCEPTION: " << e.what() << std::endl;
		peak::Library::Close();
		return -2;
	}
	catch (const peak::core::BadAccessException& e)
	{
		// The NodeAccess was not suitable for the intended task, e.g. the node was ReadOnly
		std::cout << "Read Only EXCEPTION: " << e.what() << std::endl;
		peak::Library::Close();
		return -2;
	}
	catch (const peak::core::NotAvailableException& e)
	{
		// The node is not available, camera configuration could block current availability
		std::cout << "Not Available EXCEPTION: " << e.what() << std::endl;
		peak::Library::Close();
		return -2;
	}
	catch (const peak::core::NotFoundException& e)
	{
		// The node could not be found, check misspelling of the node name
		std::cout << "Not Found EXCEPTION: " << e.what() << std::endl;
		peak::Library::Close();
		return -2;
	}
	catch (const peak::core::NotImplementedException& e)
	{
		// The node is not implemented
		std::cout << "Not Implemented EXCEPTION: " << e.what() << std::endl;
		peak::Library::Close();
		return -2;
	}
	catch (const std::exception& e)
	{
		// all other exceptions
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
