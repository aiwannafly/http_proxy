#include "proxy_tests.h"

#include <chrono>
#include <curl/curl.h>
#include <gtest/gtest.h>
#include <string>
#include "pthread.h"

#define USAGE_GUIDE "usage: ./tests <proxy_port>"

#define TEST_FILES_URL "www.ccfit.nsu.ru/~rzheutskiy/test_files"
#define DATA_50MB_URL "www.ccfit.nsu.ru/~rzheutskiy/test_files/50mb.dat"
#define DATA_100MB_URL "www.ccfit.nsu.ru/~rzheutskiy/test_files/100mb.dat"
#define DATA_200MB_URL "www.ccfit.nsu.ru/~rzheutskiy/test_files/200mb.dat"
#define DATA_500MB_URL "www.ccfit.nsu.ru/~rzheutskiy/test_files/500mb.dat"

namespace {
    const int REQUIRED_ARGC = 1 + 1;

    typedef struct args_t {
        bool valid;
        int proxy_port;
    } args_t;

    bool ExtractInt(const char *buf, int *num) {
        if (nullptr == buf || num == nullptr) {
            return false;
        }
        char *end_ptr = nullptr;
        *num = (int) strtol(buf, &end_ptr, 10);
        if (buf + strlen(buf) > end_ptr) {
            return false;
        }
        return true;
    }

    args_t ParseArgs(int argc, char *argv[]) {
        args_t result;
        result.valid = false;
        if (argc < REQUIRED_ARGC) {
            return result;
        }
        bool extracted = ExtractInt(argv[1], &result.proxy_port);
        if (!extracted) {
            return result;
        }
        result.valid = true;
        return result;
    }

    size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
        ((std::string *) userp)->append((char *) contents, size * nmemb);
        return size * nmemb;
    }

    size_t port;

    void log(const std::string &s) {
        std::cout << "[   LOG    ] " << s << std::endl;
    }

    void logErr(const std::string &s) {
        std::cerr << "[   LOG    ] " + s + "\n";
    }

    typedef struct download_info {
        pthread_t tid{};
        std::string buffer;
        CURLcode code = CURLE_READ_ERROR;
        bool with_proxy = true;
        long timeout_secs = 0;
        std::string url;
    } download_info;

    void GetData(const std::string &url, bool with_proxy, std::string *buffer, CURLcode *res,
                 long timeout_secs) {
        assert(buffer);
        assert(res);
        CURL *curl;
        curl = curl_easy_init();
        if (curl == nullptr) return;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
        curl_easy_setopt(curl, CURLOPT_URL, url.data());
        if (with_proxy) {
            curl_easy_setopt(curl, CURLOPT_PROXY, ("http://localhost:" + std::to_string(port)).data());
        }
        if (timeout_secs > 0) {
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_secs);
        }
        *res = curl_easy_perform(curl);
        if (*res != CURLE_OK) {
            logErr(curl_easy_strerror(*res));
        }
        curl_easy_cleanup(curl);
    }

    void *DownloadStart(void *arg) {
        auto *info = (struct download_info *) arg;
        GetData(info->url, info->with_proxy, &info->buffer, &info->code, info->timeout_secs);
        return nullptr;
    }
}

TEST(HTTP_PROXY, BaseTest) {
    CURLcode res;
    std::string read_buffer;
    GetData(TEST_FILES_URL, true, &read_buffer, &res, 0);
    ASSERT_EQ(res, CURLE_OK);
}

// TEST(HTTP_PROXY, ProxyDelayTest) {
//     size_t iter_count = 5;
//     size_t total_millis = 0;
//     std::string read_buffer;
//     CURLcode res;
//     auto start = std::chrono::steady_clock::now();
//     GetData(DATA_50MB_URL, true, &read_buffer, &res, 0);
//     auto end = std::chrono::steady_clock::now();
//     ASSERT_EQ(res, CURLE_OK);
//     size_t elapsed_millis = duration_cast<std::chrono::milliseconds>(end - start).count();
//     log("Download time with proxy: " + std::to_string(elapsed_millis) + " millis");
//     for (size_t i = 0; i < iter_count; i++) {
//         start = std::chrono::steady_clock::now();
//         GetData(DATA_50MB_URL, false, &read_buffer, &res, 0);
//         end = std::chrono::steady_clock::now();
//         ASSERT_EQ(res, CURLE_OK);
//         total_millis += duration_cast<std::chrono::milliseconds>(end - start).count();
//     }
//     size_t avg_millis = total_millis / iter_count;
//     log("Average download time with no proxy: " + std::to_string(avg_millis) + " millis");
//     bool faster = elapsed_millis <= avg_millis;
//     size_t diff = !faster ? (elapsed_millis - avg_millis) : (avg_millis - elapsed_millis);
//     size_t diffp = 100 * diff / avg_millis;
//     log("With proxy downloaded " + std::to_string(diffp) + "% " + std::string(faster ? "faster" : "slower"));
//     EXPECT_TRUE(diffp <= 20);
// }

// TEST(HTTP_PROXY, SpeedIncreaseTest) {
//     CURLcode res;
//     std::string read_buffer1;
//     auto start = std::chrono::steady_clock::now();
//     GetData(DATA_50MB_URL, true, &read_buffer1, &res, 0);
//     auto end = std::chrono::steady_clock::now();
//     ASSERT_EQ(res, CURLE_OK);
//     size_t millis_first = duration_cast<std::chrono::milliseconds>(end - start).count();
//     std::string read_buffer2;
//     start = std::chrono::steady_clock::now();
//     GetData(DATA_50MB_URL, true, &read_buffer2, &res, 0);
//     end = std::chrono::steady_clock::now();
//     ASSERT_EQ(res, CURLE_OK);
//     size_t millis_second = duration_cast<std::chrono::milliseconds>(end - start).count();
//     EXPECT_TRUE(millis_second * 2 < millis_first);
//     if (millis_second > 1)
//         log("Completed with cache " + std::to_string(millis_first / millis_second) + " times faster");
// }

// TEST(HTTP_PROXY, MultipleConnectionsTest) {
//     size_t conns_count = 20;
//     log("Launching " + std::to_string(conns_count) + " 100MB-download sessions through PROXY");
//     ASSERT_FALSE(conns_count <= 0);
//     auto download_segments = std::vector<download_info *>();
//     for (size_t i = 0; i < conns_count; i++) {
//         auto info = new download_info();
//         if (i == 0) {
//             info->with_proxy = false;
//         }
//         info->url = DATA_100MB_URL;
//         download_segments.push_back(info);
//         int code = pthread_create(&info->tid, nullptr, DownloadStart, info);
//         EXPECT_FALSE(code < 0);
//     }
//     for (const auto &info: download_segments) {
//         int code = pthread_join(info->tid, nullptr);
//         EXPECT_FALSE(code < 0);
//     }
//     auto canonic_buffer = download_segments.front()->buffer;
//     for (const auto &info: download_segments) {
//         EXPECT_EQ(info->code, CURLE_OK);
//         EXPECT_EQ(canonic_buffer, info->buffer);
//         delete info;
//     }
// }

TEST(HTTP_PROXY, RotationTest) {
    size_t rotations_count = 5;
    size_t conns_count = 10;
    std::string url = DATA_100MB_URL;

    std::string read_buffer0;
    CURLcode res;
    log("Launching rotation test with multiple 100MB-download sessions");
    auto start = std::chrono::steady_clock::now();
    GetData(url, false, &read_buffer0, &res, 0);
    auto end = std::chrono::steady_clock::now();
    size_t total_time_millis = duration_cast<std::chrono::milliseconds>(end - start).count();
    log("Downloading time without proxy : " + std::to_string(total_time_millis / 1000) + " seconds");
    size_t time_chunk_millis = total_time_millis / rotations_count;
    ASSERT_EQ(res, CURLE_OK);
    ASSERT_TRUE(total_time_millis > 0);
    auto lead_segments = std::vector<download_info *>();
    for (size_t j = 0; j < rotations_count; j++) {
        auto download_segments = std::vector<download_info *>();
        size_t lead_idx = 0;
        log("Initiate new rotation launch");
        for (long i = 0; i < conns_count; i++) {
            auto info = new download_info();
            info->url = url;
            info->timeout_secs = time_chunk_millis / 1000;
            if (i == lead_idx) {
                info->timeout_secs *= 2; // get first file without proxy
                if (j == rotations_count - 1) {
                    info->timeout_secs = 0;
                }
                lead_segments.push_back(info);
            } else {
                download_segments.push_back(info);
            }
            int code = pthread_create(&info->tid, nullptr, DownloadStart, info);
            EXPECT_FALSE(code < 0);
        }
        start = std::chrono::steady_clock::now();
        for (const auto &info : download_segments) {
            int code = pthread_join(info->tid, nullptr);
            EXPECT_FALSE(code < 0);
            EXPECT_TRUE(info->code == CURLE_OPERATION_TIMEDOUT || info->code
                                                                  == CURLE_OK);
            if (info->code == CURLE_OK) {
                EXPECT_EQ(read_buffer0, info->buffer);
            }
            delete info;
        }
        end = std::chrono::steady_clock::now();
        size_t rot_millis = duration_cast<std::chrono::milliseconds>(end - start).count();
        log("Rotation " + std::to_string(j + 1) + " ended in " + std::to_string(rot_millis / 1000) + " seconds");
    }
    size_t count = 0;
    for (const auto &info : lead_segments) {
        count++;
        int code = pthread_join(info->tid, nullptr);
        EXPECT_FALSE(code < 0);
        if (count == rotations_count) {
            EXPECT_EQ(info->code, CURLE_OK);
            EXPECT_EQ(read_buffer0, info->buffer);
        } else {
            EXPECT_TRUE(info->code == CURLE_OPERATION_TIMEDOUT || info->code
                                                                  == CURLE_OK);
            if (info->code == CURLE_OK) {
                ASSERT_EQ(read_buffer0, info->buffer);
            }
        }
        delete info;
    }
}

int RunAllTests(int argc, char *argv[]) {
    args_t args = ParseArgs(argc, argv);
    if (!args.valid) {
        fprintf(stderr, "%s\n", USAGE_GUIDE);
        return -1;
    }
    log("Using " + std::string(TEST_FILES_URL) + " for testing...");
    port = args.proxy_port;
    CURLcode res;
    std::string read_buffer;
    GetData(TEST_FILES_URL, true, &read_buffer, &res, 0);
    if (res != CURLE_OK) {
        log("Could not connect to proxy on localhost:" +  std::to_string(port));
        return -1;
    }
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
