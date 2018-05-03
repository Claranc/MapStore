#include <iostream>
#include <fstream>
#include <string>
#include <map>
using namespace std;

#define SIZE 100000

//输出到多个文件的类
class MultifulFilesWriting {
    public:
        const char* fin_file_name;
        string linestr;
        string key;
        string value;
        int value_length;
        int count = 0;
        int file_flag = 0;
        int file_num = 0;
        static string key_file_name;
        static string value_file_name;
        int offset = 0;

    public:
        //构造函数
        MultifulFilesWriting(const char* srcname) {
            fin_file_name = srcname;
        }

        void WriteToFiles() {
            ifstream fin(fin_file_name, ios::in);
            while(getline(fin, linestr)) {
                if(!linestr.size()) continue;
                count++;
                int a = linestr.find_first_of(" ");
                key = linestr.substr(0,a);
                value = linestr.substr(a+1);
                file_num = count/SIZE + 1;
                value_length = value.size();
                
                ofstream fout_key(key_file_name, ios::app);
                ofstream fout_value(value_file_name, ios::app);

                //输出到key文件中
                fout_key << key << " ";
                fout_key.write((char*)&offset, sizeof(int));
                fout_key.write((char*)&value_length, sizeof(int));
                //输出到value文件中
                fout_value << value << "\n";
                offset += (value_length + sizeof(char));
                key.clear();
                value.clear();
                //判断是否超过SIZE,超过则变成对应的文件名
                if(file_num - file_flag != 1 ) {
                    file_flag++;
                    key_file_name = "ps_00" + to_string(file_num) + "_key.bin";
                    value_file_name = "ps_00" + to_string(file_num) + "_value.bin";
                    offset = 0;
                }   
        }
    }
};

string MultifulFilesWriting::key_file_name = "ps_001_key.bin";
string MultifulFilesWriting::value_file_name = "ps_001_value.bin";

//参数1为数据文件名字
int main(int argc, char* argv[]) {
    MultifulFilesWriting A(argv[1]);
    A.WriteToFiles();
    return 0;
}
