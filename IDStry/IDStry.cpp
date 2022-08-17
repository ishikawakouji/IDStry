// IDStry.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iostream>

#include <peak/converters/peak_buffer_converter_ipl.hpp>
#include <peak/peak.hpp>
#include <peak_ipl/peak_ipl.hpp>

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



		// pixel format
		std::string format = nodeMapRemoteDevice->FindNode<peak::core::nodes::EnumerationNode>("PixelFormat")->CurrentEntry()->SymbolicValue();
		cout << "pixel format " << format << endl;

		// acquisition準備
		auto dataStreams = device->DataStreams();
		if (dataStreams.empty()) {
			cout << "no data stream" << endl;
			return -1;
		}

		auto dataStream = dataStreams.at(0)->OpenDataStream();

		// 古いバッファを消す
		dataStream->Flush(peak::core::DataStreamFlushMode::DiscardAll);
		for (const auto& buffer : dataStream->AnnouncedBuffers()) {
			dataStream->RevokeBuffer(buffer);
		}

		// payload サイズ
		int64_t payloadSize = nodeMapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("PayloadSize")->Value();

		// 必要なバッファの数
		size_t mumBuffersMinRequired = dataStream->NumBuffersAnnouncedMinRequired();
		cout << mumBuffersMinRequired << " buffer requiered" << endl;

		// バッファ確保
		for (size_t count = 0; count < mumBuffersMinRequired; ++count) {
			auto buffer = dataStream->AllocAndAnnounceBuffer(payloadSize, nullptr);
			dataStream->QueueBuffer(buffer);
		}

		// まずはフリーラン
		nodeMapRemoteDevice->FindNode<peak::core::nodes::EnumerationNode>("TriggerSelector")->SetCurrentEntry("ExposureStart");
		nodeMapRemoteDevice->FindNode<peak::core::nodes::EnumerationNode>("TriggerMode")->SetCurrentEntry("Off");
		
		// フレームレートを設定する
		// set a frame rate to 10fps (or max value) since some of the trigger cases require a defined frame rate
		auto frameRateMax = nodeMapRemoteDevice->FindNode<peak::core::nodes::FloatNode>("AcquisitionFrameRate")->Maximum();
		nodeMapRemoteDevice->FindNode<peak::core::nodes::FloatNode>("AcquisitionFrameRate")->SetValue(std::min(10.0, frameRateMax));

		// define the number of images to acquire
		uint64_t imageCountMax = 3;

		// Lock critical features to prevent them from changing during acquisition
		nodeMapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("TLParamsLocked")->SetValue(1);

		// start acquisition
		dataStream->StartAcquisition(peak::core::AcquisitionStartMode::Default, imageCountMax);
		nodeMapRemoteDevice->FindNode<peak::core::nodes::CommandNode>("AcquisitionStart")->Execute();

		// process the acquired images
		uint64_t imageCount = 0;
		char index = '0';
		std::cout << std::endl << "First pixel value of each image: " << std::endl;
		while (imageCount < imageCountMax)
		{
			// get buffer from datastream and create IDS peak IPL image from it
			auto buffer = dataStream->WaitForFinishedBuffer(5000);
			auto image = peak::BufferTo<peak::ipl::Image>(buffer); // なにもしなければ BayerRG8

			// output first pixel value
			std::cout << static_cast<uint16_t>(*image.PixelPointer(0, 0)) <<
				" IPL pixcel info " << image.PixelFormat().Name() <<
				" " << image.ByteCount() << " " << image.Width() << " x " << image.Height() << endl;

			// save file
			string filepath = "hgoe_";
			filepath += index;
			filepath +=".png";
			peak::ipl::ImageWriter::Write(filepath, image);
			

			// queue buffer
			dataStream->QueueBuffer(buffer);
			++imageCount;
			++index;
		}
		std::cout << std::endl << std::endl;

		// stop acquistion of camera
		try
		{
			dataStream->StopAcquisition(peak::core::AcquisitionStopMode::Default);
		}
		catch (const std::exception&)
		{
			// Some transport layers need no explicit acquisition stop of the datastream when starting its
			// acquisition with a finite number of images. Ignoring Errors due to that TL behavior.

			std::cout << "WARNING: Ignoring that TL failed to stop acquisition on datastream." << std::endl;
		}
		nodeMapRemoteDevice->FindNode<peak::core::nodes::CommandNode>("AcquisitionStop")->Execute();

		// Unlock parameters after acquisition stop
		nodeMapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("TLParamsLocked")->SetValue(0);

		// flush and revoke all buffers
		dataStream->Flush(peak::core::DataStreamFlushMode::DiscardAll);
		for (const auto& buffer : dataStream->AnnouncedBuffers())
		{
			dataStream->RevokeBuffer(buffer);
		}

		/*

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
		*/

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
