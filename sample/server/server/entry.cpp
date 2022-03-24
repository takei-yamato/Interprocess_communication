#include <iostream>
#include <winsock2.h>
#include <array>
#include <thread>
#include <mutex>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

// 同期オブジェクト
std::mutex	mtx = {};

// 受信に利用する情報
struct Recive {
    SOCKET                         socket = {};	// 接続時に生成されたこの接続用のソケット識別子
    std::vector<std::string> data = {};	// 受信データ
};

// -----------------------------------------------------------
// メイン関数
// -----------------------------------------------------------
int main()
{
    WSADATA wsaData = {};
    // winsock 利用を開始する
    if (WSAStartup(MAKEWORD(2, 0), &wsaData) == 0) {

        // ソケットを作成する( IPv4 通信 / TCP を利用する )
        auto sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) {
            std::cout << "ソケットの作成に失敗しました" << std::endl;
            return 1;
        }

        // ソケットを紐付ける
        SOCKADDR_IN addrIn = {};
        addrIn.sin_family = AF_INET;		// IPv4 通信
        addrIn.sin_port = htons(8080);	// ポート番号（※任意のポート番号に変更してください）
        addrIn.sin_addr.S_un.S_addr = INADDR_ANY;	// どのアドレスからでも受け入れる
        if (bind(sock, (PSOCKADDR)(&addrIn), sizeof(addrIn)) == SOCKET_ERROR) {
            std::cout << "ソケットの紐付けに失敗しました" << std::endl;
            return 2;
        }

        // 接続を待つ（接続待機は一つだけ）
        if (listen(sock, 1) == SOCKET_ERROR) {
            std::cout << "ソケットの接続準備に失敗しました" << std::endl;
            return 3;
        }
        std::cout << "ソケットの接続準備完了" << std::endl;


        // クライアントとの接続情報を保持する
        Recive recive;

        // 接続してきたクライアント情報の格納先を用意する
        SOCKADDR_IN	clientInfo = {};
        auto		infoSize = static_cast<int>(sizeof(clientInfo));

        // 接続要求を許可する
        recive.socket = accept(sock, (PSOCKADDR)(&clientInfo), &infoSize);
        if (recive.socket == INVALID_SOCKET) {
            std::cout << "接続が許可されませんでした" << std::endl;
            return 4;
        }
        std::cout << "接続完了" << std::endl;

        // 一旦接続が切れたらメインループを終了する
        bool finish = {};

        // 受信ワーカーを別スレッドで処理する
        std::thread worker([&]() {
            while (true) {
                // データの格納先
                std::array<char, 256> buf = {};
                // 受信するまで待つ
                auto res = recv(recive.socket, buf.data(), buf.size(), 0);

                // エラーあるいは通常終了の場合、このスレッドを終える
                auto error = (res == SOCKET_ERROR || res == 0);

                // データ競合を防ぐ
                {
                    std::lock_guard<decltype(mtx)> lock(mtx);
                    recive.data.emplace_back(buf.data());
                    finish = error;
                }

                if (error) { break; }
            }
            });

        // メインループ
        while (true) {

            bool                     endLoop = {};
            std::vector<std::string> data = {};

            // データ競合を防ぐ
            {
                std::lock_guard<decltype(mtx)> lock(mtx);
                data.swap(recive.data);
                endLoop = finish;
            }

            // 文字を表示する
            for (const auto& s : data) {
                std::cout << s << std::endl;
            }

            // メインループを抜ける
            if (endLoop) { break; }

            // 一秒待つ
            Sleep(1000);
        }

        // 受信ワーカーの終了を待つ
        worker.join();

        // ソケット通信を終了する
        closesocket(sock);
    }

    // Winsockを終了
    WSACleanup();

    return 0;
}