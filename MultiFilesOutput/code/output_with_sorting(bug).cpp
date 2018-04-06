#include <iostream>
#include <fstream>
#include <string>
#include <map>
using namespace std;

#define SIZE 30

//文件内容排序
        class OrderlyFiles {
    public:
        int num;
        string key;
        string value;
        map<string,string> mymap;
        int m,n;
        int i,j,k,p;
        int offset,value_length;
        static string file_key_name;
        static string file_value_name;
    public:
        OrderlyFiles(int l) {
            num = l;
        }
        
        //将第i个文件的map传入内存中
        void ExtractDataToMap(int i) {
            file_key_name = "ps_00" + to_string(i) + "_key.bin";
            file_value_name = "ps_00" + to_string(i) + "_value.bin";
            ifstream fin_key(file_key_name, ios::in);
            ifstream fin_value(file_value_name, ios::in);
            fin_key.seekg(0, ios::end);
            n = fin_key.tellg();
            char *kbuff = new char[n];
            fin_key.seekg(0, ios::beg);
            fin_key.read((char*)kbuff,n);
            j = 0;
            while(j < n) {
                if(kbuff[j] != ' ') key.push_back(kbuff[j++]);
                else {
                    offset = *((int*)&kbuff[++j]);
                    j += 4;
                    value_length = *((int*)&kbuff[j]);
                    fin_value.seekg(offset, ios::beg);
                    char *vbuff = new char[value_length];
                    fin_value.read(vbuff,value_length);
                    for(k = 0; k < value_length; k++) value.push_back(vbuff[k]);
                    if(key.size() != 0 && value.size() != 0) { 
                        mymap.insert(pair<string,string>(key,value));
                    }
                    key.clear();
                    value.clear();
                    delete[] vbuff;
                    j+=4;
                }
            }
            fin_key.close();
            fin_value.close();
            delete[] kbuff;
        }
        
        //将内存中的一部分map输出到文件x中
        void OutputToFiles(int x) {
            int sum = 0;
            int offset = 0;
            int value_length;
            file_key_name = "ps_00" + to_string(x) + "_key.bin";
            file_value_name = "ps_00" + to_string(x) + "_value.bin";
            ofstream fout_key_resort(file_key_name, ios::out);
            ofstream fout_value_resort(file_value_name, ios::out);
            map<string,string>::iterator it = mymap.begin();
            for(; it != mymap.end(); it++) {
                fout_key_resort << it->first << " ";
                value_length = it->second.size();
                fout_key_resort.write((char*)&offset,sizeof(int));
                fout_key_resort.write((char*)&value_length,sizeof(int));
                fout_value_resort << it->second << endl;
                offset+=(value_length+sizeof(char));
                sum++;
                if(sum == SIZE) {
                    it++;
                    break;
                }
            }
            fout_key_resort.close();
            fout_value_resort.close();
            mymap.erase(mymap.begin(), it);
        }
        
        void SortFiles() {
            ExtractDataToMap(num);
            for(p = 1; p <= num - 1; p++) {
                ExtractDataToMap(p);
            	OutputToFiles(p);
            }
            OutputToFiles(num);
        }
};      
string OrderlyFiles::file_key_name = "ps_001_key.bin";
string OrderlyFiles::file_value_name = "ps_001_value.bin";

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
            ofstream fout_key(key_file_name, ios::out);
            ofstream fout_value(value_file_name, ios::out);
            while(getline(fin, linestr)) {
                if(!linestr.size()) continue;
                count++;
                int a = linestr.find_first_of(" ");
                key = linestr.substr(0,a);
                value = linestr.substr(a+1);
                file_num = count/SIZE + 1;
                value_length = value.size();
                //输出到key文件中
                fout_key << key << " ";
                fout_key.write((char*)&offset, sizeof(int));
                fout_key.write((char*)&value_length, sizeof(int));
                //输出到value文件中
                fout_value << value << "\n";
                offset += (value_length + sizeof(char));
                key.clear();
                value.clear();
                if(count%SIZE == 0 && count/SIZE > 0) {
                    fout_key.close();
                    fout_value.close();
                    OrderlyFiles* rec = new OrderlyFiles(count/SIZE);
                    rec->SortFiles();
                    delete rec;
                    key_file_name = "ps_00" + to_string(file_num) + "_key.bin";
                    value_file_name = "ps_00" + to_string(file_num) + "_value.bin";
                    fout_key.open(key_file_name, ios::out);
                    fout_value.open(value_file_name, ios::out);
                }
                //判断是否超过SIZE,超过则变成对应的文件名
                /*if(file_num - file_flag != 1 ) {
                    fout_key.close();
                    fout_value.close();
                    file_flag++;
                    key_file_name = "ps_00" + to_string(file_num) + "_key.bin";
                    value_file_name = "ps_00" + to_string(file_num) + "_value.bin";
                    fout_key.open(key_file_name, ios::out);
                    fout_value.open(value_file_name, ios::out);
                    offset = 0;
                }*/   
            }
            fout_key.close();
            fout_value.close();
            //将最后一个文件排序
            /*OrderlyFiles* rec = new OrderlyFiles(count/SIZE+1);
            rec->SortFiles();
            delete rec;*/
        }
    };

string MultifulFilesWriting::key_file_name = "ps_001_key.bin";
string MultifulFilesWriting::value_file_name = "ps_001_value.bin";


int main(int argc, char* argv[]) {
    MultifulFilesWriting A(argv[1]);
    A.WriteToFiles();
    return 0;
}
    
