/*
This program generates a hash table of every combination of characters of a specified length, and outputs the data to OutFile.csv
Optional salt can be added, and there are 4 settings for what characters are included:
  0 is lowercase
  1 is uppercase + lowercase
  2 is numbers + upper + lower
  3 is symbols + numbers + upper + lower

  I put a safety limit of 10GB on the output file, anything past that will not run.
    This can be disabled inside of the genRbTable function, with how the code runs now, the filesize estimate is pretty accurate.
	But it won't be accurate if the output data code is modified.


	SHA256 hashing library created by Stephan Brumme. 
	Link to his website: https://create.stephan-brumme.com/hash-library/
	Link to his Github:  https://github.com/stbrumme/hash-library
*/

#include<iostream>
#include<iomanip>
#include<fstream>
#include<string>
#include<math.h>
#include"sha256.h" //Used a library for hashing, this allows me to put data into a sha256() function and it returns a string with the hash.

using namespace std;

void genRbTable(int combLen, string salt, int setting, ofstream& outFile, SHA256& sha256);
void calcRBTable(string chars, int charsLen, int combLen, string salt, ofstream& outFile, SHA256& sha256);

int main()
{
	SHA256 sha256;
	ofstream outFile("OutFile.csv");

	//First is how many characters long the combinations should be, next is the salt, the setting, next is the output file, and finally is the hashing object.
	//Setting: 0 is lowercase, 1 is uppercase + lowercase, 2 is numbers + upper + lower, 3 is symbols + numbers + upper + lower.
	genRbTable(2, "n", 0, outFile, sha256);
	outFile.close();

	return 0;
}

void genRbTable(int combLen, string salt, int setting, ofstream& outFile, SHA256& sha256)
{
	//Add characters to the combination string based on the setting
	string chars = "";
	switch (setting)
	{
	case(3):
		chars = ".,!$#*" + chars;
	case(2):
		chars = "0123456789" + chars;
	case(1):
		chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" + chars;
	case(0):
		chars = "abcdefghijklmnopqrstuvwxyz" + chars;
	}
	
	//Calculate the filesize and warn user how big it will be
	uint64_t fileSize = 68 + combLen + salt.length(); //68 is the number used for commas, hash characters, and the CRLF chars
	fileSize = fileSize * pow(chars.length(), combLen); //calculate how many lines will be generated
	float roundedSz = fileSize / static_cast<float>(1000000);

	cout << "This will take " << fileSize << " bytes to store. Continue? (Y/N)" << endl;
	cout << "\t" << setprecision(3) << fixed << roundedSz << "MB" << endl;

	char input;
	cin >> input;
	
	if (input == 'y' || input == 'Y')
	{
		if (fileSize < 10000000000) //if less than 10 GB
		{
			//Do the calculation
			calcRBTable(chars, chars.length(), combLen, salt, outFile, sha256);
		}
		else
		{
			//Failsafe to make sure it doesn't get out of control.
			cout << "filesize exceeds 10GB, I'm not doing that." << endl;
		}
	}
}

void calcRBTable(string chars, int charsLen, int combLen, string salt, ofstream& outFile, SHA256& sha256)
{
	//Create an array to store the status of each character in the combination
	int* output = new int[combLen];
	for (int i = 0; i < combLen; i++)
	{
		output[i] = 0;
	}


	//Think of this next section like an old physical clicker counter. the counter has 000 to start,
	// then, you click the button and it switches to 001, then 002, ... , 009, 010, 011, etc.
	// This is how the combinations are calculated. instead of 0 through 9 on the dial, it's each element of the chars string.
	// when the rightmost dial increments above charsLen, reset it to 0, then increment the next dial by 1.
	// Stop when every dial is equal to charsLen.

	bool looping = false;
	do
	{
		if (looping) //Increment the counter
		{
			for (int i = combLen - 1; i >= 0; i--) //start from the last index and work forward
			{
				output[i] += 1; //Increment the current dial by 1,
				if (output[i] < charsLen) //if it isn't above its max value, no more work is needed.
				{
					break;
				}
				else //If it is above the max value, reset it and run through the loop again with the next dial.
				{
					output[i] = 0; 
				}
			}
		}

		string temp = "";
		for (int i = 0; i < combLen; i++)
		{
			temp = temp + chars[output[i]]; //Create the character combination that correlates to output[]'s current 'dial' settings
		}
		outFile << temp << ',' << salt << ','; //write the current data to the output file, adding formatting and listing the salt
		temp = salt + temp;
		outFile << sha256(temp) << endl; //hash it and store it.

		for (int i = 0; i < combLen; i++) //Now, check if all the dials are at their max value.
		{
			looping = false;
			if (output[i] != charsLen - 1)
			{
				looping = true;
				break;
			}
		}
	} while (looping);
	delete[] output;
}