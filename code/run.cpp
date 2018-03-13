#include "Header.h"

int run(map<string, string>& mymap, string x)
{
	ofstream ous("../data/data.csv",ios::app);
	ifstream ins("../data/data.csv");
	vector<vector<string> > strArray; 
	string value;

	if(!ins) 
		{cout<<"Map file cannot be open!!!"<<endl;
		return 0;}

	while(getline(ins,value))
    {  
    	std::vector<string> linestr;
    	get_values(value,linestr,1);
    	strArray.push_back(linestr);
	    /*stringstream ss(value);
	    string str;  
	    vector<string> lineArray;  
	    while (getline(ss, str, ','))  
	            lineArray.push_back(str);  
	    strArray.push_back(lineArray);*/
    }
    for(int i=0;i<strArray.size();i++)
    {
    	if(strArray[i].size() == 2 )mymap.insert(pair<string,string>(strArray[i][0],strArray[i][1]));
    	else cout<<"Line "<< i+1 <<" has an error"<<endl;
    }

	while (1)
	{
        cout<<">> ";
		getline(cin, x);
		vector<string> vals;
		if (x == "quit")
			break;
		int i = choose(x);
		int k;
		switch (i)
		{
		case 0:
			cout << "Your input is wrong!" << endl;
			break;
		case 1:
			k = get_values(x, vals,2);
			if (k == 0 && vals.size() == 3)
				{int l = create_key(mymap, vals[1], vals[2]);
				if(l == 0 ) ous << vals[1] <<","<< vals[2] << endl;}
			else cout << "Your parameter is not correct!" << endl;
			break;
		case 2:
			k = get_values(x, vals,2);
			if (k == 0 && vals.size() == 3)update_key(mymap, vals[1], vals[2]);
			else cout << "Your parameter is not correct!" << endl;
			break;
		case 3:
			k = get_values(x, vals,2);
			if (k == 0 && vals.size() == 2)delete_key(mymap, vals[1]);
			else  cout << "Your parameter is not correct!" << endl;
			break;
		case 4:
			k = get_values(x, vals,2);
			if (k == 0 && vals.size() == 2)cout << get_key(mymap, vals[1]) << endl;
			else cout << "Your parameter is not correct!" << endl;
			break;
		case 5:
			cout << "You can \"create\",\"update\",\"delete\",\"get\",\"quit\",\"show\""
                    <<endl;
            continue;
            break;
        case 6:
        	show_keys(mymap);
        	break;
		}

	}

	ofstream ous2("../data/data.csv");
	for(map<string,string>::iterator it = mymap.begin(); it != mymap.end();it++)
        {
            ous<<it->first<<" "<<"\""<<it->second<<"\""<<endl;
        }
}
