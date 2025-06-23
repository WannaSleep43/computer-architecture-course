#include <iostream>
#include <vector>
#include <string>

using namespace std;

#define INF(a, b) (((0 << a) + ((1ll << a) - 1)) << b);
#define MINF(a, b) (((1 << a) + ((1ll << a) - 1)) << b);

#define NAN(a, b) (INF(a, b) + 1);

long long roundingType = 1;

vector<long long> parseString(string & arg) {
    vector < string > data;
    int l = 0, r = 0;
    while (l < arg.length()) {
        char c = ' ';
        while (r < arg.length() && arg[r] != (const char) ' ' && arg[r] != (const char) '.') r++;
        data.push_back(arg.substr(l, r - l));
        l = ++r;
    }

	long long add = (data[0] == "h" || data[0] == "f") ? 0 : 1;

	if (data.size() != 3 + add && data.size() != 5 + add) {
		cerr << "Wrong number of arguments.\n" << "expected " << 3 << " or " << 5 << "\n" << "actual: " << data.size() - add << '\n';
		exit(3);
	}

    vector<long long> result(data.size(), 0);
    if (data[0] != "h" && data[0] != "f") result[0] = stoll(data[1]);
    if (data[0] == "h") result[0] = -1;
    if (data[0] == "f") result[0] = -2;

    result[1] = stoll(data[1 + add]);
    result[2] = stoll(data[2 + add].substr(2), nullptr, 16);

    if (data.size() > 3 + add && data[3 + add] == "+") result[3] = 0;
    if (data.size() > 3 + add && data[3 + add] == "-") result[3] = 1;
    if (data.size() > 3 + add && data[3 + add] == "*") result[3] = 2;
    if (data.size() > 3 + add && data[3 + add] == "/") result[3] = 3;
    if (data.size() > 3 + add) result[4] = stoll(data[4 + add].substr(2), nullptr, 16);
	if (data.size() % 2 == 0) result[result.size() - 1] =  stoll(data[0]);
    return result;
}

string itos(long long x) {
	string res = "";
	if (x == 0) res = string(17, '0');
	while (x) {
		char c = '0' + (x % 10);
		res = c + res;
		x /= 10;
	}
	return res;
}


int kthBit(long long x, long long k) {
	return (x >> k) & 1;
}

long long findFirstOne(long long x) {
	long long first = 63; 
	if (x == 0) return -1;
	while (kthBit(x, first) != 1) first--;
	return first;
}

long long toAddition(long long x, long long bitCount) {
	return (x ^ ((1ll << bitCount) - 1)) + 1;
}

long long rounding(long long value, long long bitsToRemove, bool sign) {
	long long roundedDownValue = value >> min(63ll, bitsToRemove);
	if (bitsToRemove > 63) roundedDownValue = 0;
	long long roundedUpValue = roundedDownValue + 1;
	long long toRound;
	if (bitsToRemove > 63) toRound = value;
	else toRound = value & ((1ll << bitsToRemove) - 1);

	bitsToRemove = min(63ll, bitsToRemove);
	if (!toRound) return roundedDownValue;

	if (roundingType == 0) {
		return roundedDownValue;
	} else if (roundingType == 1) {
		if (toRound > (1ll << (bitsToRemove - 1))) return roundedUpValue;
		if (toRound == (1ll << (bitsToRemove - 1)) && (roundedDownValue % 2)) return roundedUpValue;
		return roundedDownValue;
	} else if (roundingType == 2) {
		if (sign) return roundedDownValue;
		return roundedUpValue;
	} else {
		if (!sign) return roundedDownValue;
		return roundedUpValue;
	}
}

pair<string, bool> rounding(string&value, long long bitCount, bool sign) {
	int cnt = value.size() - bitCount;
	string roundedDownValue = value.substr(0, cnt);
	string roundedUpValue = itos(stoll(roundedDownValue) + 1);
	if (roundedDownValue.size() > roundedUpValue.size())
		roundedUpValue = string(roundedDownValue.size() - roundedUpValue.size(), '0') + roundedUpValue;

	pair<string, bool> ansDown = {roundedDownValue, 0}, ansUp = {roundedUpValue, 0};

	if (roundedUpValue.size() > roundedDownValue.size()) {
		ansUp.second = 1;
		ansUp.first = string(cnt, '0');
	}

	string toRound = value.substr(value.size() - bitCount, bitCount);
	string toEqual = "5" + string(bitCount - 1, '0');

	if (toRound == string(bitCount, '0')) return ansDown;

	if (roundingType == 0) {
		return ansDown;
	} else if (roundingType == 1) {
		if (toRound > toEqual) return ansUp;
		if (toRound == toEqual && (roundedDownValue[roundedDownValue.size() - 1] - '0') % 2) return ansUp;
		return ansDown;
	} else if (roundingType == 2) {
		if (sign) return ansDown;
		return ansUp;
	} else {
		if (!sign) return ansDown;
		return ansUp;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

pair<string, bool> binToDecFract(long long x, long long bitCount, bool sign) {
	long long result = 0, copy;
	long long add = 5'000'000'000'000'000'0;
	
	for (int i = bitCount - 1; i >= 0; i--) {
		if ((x >> i) & 1) result += add;
		add /= 2;
	}

	string strRes = itos(result);
	strRes = string(17 - strRes.size(), '0') + strRes;
	return rounding(strRes, 14, sign);
}

long long sumFixed(long long a, long long b, long long whole, long long fract) {
    return (a + b) & ((1ll << (whole + fract)) - 1);
}

long long subFixed(long long a, long long b, long long whole, long long fract) {
    return (a + toAddition(b, fract + whole)) & ((1ll << (whole + fract)) - 1);
}

long long multiplyFixed(long long a, long long b, long long whole, long long fract) {
	int sign = 0;
	if (kthBit(a, whole + fract - 1) == 1) sign ^= 1, a = toAddition(a, whole + fract);
	if (kthBit(b, whole + fract - 1) == 1) sign ^= 1, b = toAddition(b, whole + fract);

	long long result = rounding(a * b, fract, sign);
	result = result & ((1ll << (whole + fract)) - 1);
	if (sign) result = toAddition(result, whole + fract);
    return result;
}

long long divisionFixed(long long a, long long b, long long whole, long long fract) {
	int sign = 0;
	if (kthBit(a, whole + fract - 1) == 1) sign ^= 1, a = toAddition(a, whole + fract);
	if (kthBit(b, whole + fract - 1) == 1) sign ^= 1, b = toAddition(b, whole + fract);

    a = (a << (fract + 15));

    long long result = rounding(a / b, 15, sign);
	result = result & ((1ll << (whole + fract)) - 1);
	if (sign) result = toAddition(result, whole + fract);

    return result;
}

void printFixed(long long res, long long whole, long long fract) {
	res = res & ((1ll << (whole + fract)) - 1);
	long long addition = res;

	bool sign = false;
	if (kthBit(res, (whole + fract) - 1) == 1) {
		cout << "-";
		sign = true;
		addition = toAddition(res, whole + fract);
	}
	long long wholeResult = addition >> fract;
	pair<string, bool> roundedFract = binToDecFract(addition & (((1ll << fract) - 1)), fract, sign);
	if (roundedFract.second) wholeResult += (sign ? -1 : 1);


	cout << wholeResult << "." << roundedFract.first << '\n';
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


vector<long long> parseFloatingPoint(long long x, long long expLength, long long mantLength) {
	vector<long long> result(3);
	// sign / exp  / mantissa
	result[0] = (x >> (expLength + mantLength));
	result[1] = ((x >> mantLength) & ((1ll << expLength) - 1)) - (1ll << (expLength - 1)) + 1;
	result[2] = x & ((1ll << mantLength) - 1);

	if (result[1] == -(1ll << (expLength - 1)) + 1 && result[2] != 0) {
		int index1 = findFirstOne(result[2]);
		result[2] = result[2] << (mantLength - index1);
		result[2] = result[2] & ((1ll << mantLength) - 1);
		result[1] -= mantLength - index1 - 1;
	}
	result[2] ^= (1ll << mantLength);

	return result;
}

long long convertPartsToFloat(long long sign, long long exp, long long mant, long long expLength, long long mantLength) {
	if (exp >= (1ll << expLength) - 1) {
		if (!sign) return INF(expLength, mantLength);
		return MINF(expLength, mantLength);
	}

	if (exp <= 0) {
		mant ^= (1 << mantLength);
		mant = rounding(mant, -exp + 1, sign);
		exp = 0;
	}

	return (((sign << expLength) + exp) << mantLength) + mant;

	return 0;
}

vector<long long> convertFloatingToFixed(long long a, long long b, long long expLength, long long mantLength) {
	vector<long long> value1 = parseFloatingPoint(a, expLength, mantLength), value2 = parseFloatingPoint(b, expLength, mantLength);

	if (llabs(value1[1] - value2[1]) <= 30) {
        if (value1[1] >= value2[1]) {
            value1[2] = value1[2] << (value1[1] - value2[1]);
            value2[1] = value1[1];
        } else {
            value2[2] = value2[2] << (value2[1] - value1[1]);
            value1[1] = value2[1];
        }
        return {value1[0], value1[1], value1[2], value2[0], value2[1], value2[2]};
	}

    if (value1[1] > value2[1]) {
        return {value1[0], value1[1], value1[2], 0, 0, 0};
    }
    return {value2[0], value2[1], value2[2], 0, 0, 0};
}

long long sumFloatingPoint(long long a, long long b, long long expLength, long long mantLength) {
    vector<long long> fixed = convertFloatingToFixed(a, b, expLength, mantLength);
	
	if (fixed[0] == 1) fixed[2] = -fixed[2];
	if (fixed[3] == 1) fixed[5] = -fixed[5];

    long long res = fixed[2] + fixed[5];

	if (abs(fixed[2]) == (1 << mantLength) && abs(fixed[5]) == (1 << mantLength) && (fixed[0] == 0 || fixed[3] == 0)) {
		return 0ll;
	}
	if (abs(fixed[2]) == (1 << mantLength) && abs(fixed[5]) == (1 << mantLength)) {
		return (1 << (mantLength + expLength));
	}

	int newSign = (res < 0); 

	long long maxExp = (1ll << (expLength - 1)), minExp = -(1ll << (expLength - 1)) + 1;
	long long nullMant = (1ll << mantLength);
	if ((fixed[1] == maxExp && fixed[2] != nullMant) || (fixed[4] == maxExp && fixed[5] != nullMant) ||
		(fixed[1] == maxExp && fixed[2] == nullMant && fixed[4] == maxExp && fixed[5] == nullMant && fixed[0] != fixed[3])) {
		return (((newSign << expLength) + (1ll << expLength) - 1) << mantLength) + 1; // NAN
	}

	res = llabs(res);
	if (res == 0) return 0ll;

    long long firstOne = findFirstOne(res);
	long long mx = max(findFirstOne(llabs(fixed[2])), findFirstOne(llabs(fixed[5])));
	
	long long newExp = fixed[1] + firstOne - mx;

	if (firstOne < mantLength) res <<= mantLength - firstOne;
	else res = rounding(res, firstOne - mantLength, newSign);
	res = res ^ (1ll << mantLength);

	long long newMant = res;

    newExp = newExp + (1ll << (expLength - 1)) - 1;
    
	return convertPartsToFloat(newSign, newExp, newMant, expLength, mantLength);
}

long long multiplyFloatingPoint(long long a, long long b, long long expLength, long long mantLength) {
    vector<long long> value1 = parseFloatingPoint(a, expLength, mantLength), value2 = parseFloatingPoint(b, expLength, mantLength);
    
    long long newExp = value1[1] + value2[1];
    long long newSign = value1[0] ^ value2[0];

	long long maxExp = (1ll << (expLength - 1)), minExp = -(1ll << (expLength - 1)) + 1;
	long long nullMant = (1ll << mantLength);
	if ((value1[1] == maxExp && value1[2] != nullMant) || (value2[1] == maxExp && value2[2] != nullMant) ||
	    (value1[1] == maxExp && value1[2] == nullMant && value2[1] == minExp && abs(value2[2]) == (1 << mantLength))) {
		return (((newSign << expLength) + (1ll << expLength) - 1) << mantLength) + 1; // NAN
	}

    long long res = value1[2] * value2[2];
    
    long long firstOne = findFirstOne(res);

	res = res ^ (1ll << firstOne);
    if (res == 0) return newSign << (mantLength + expLength);

	long long newMant = rounding(res, max(0ll, firstOne - mantLength), newSign);

    if (firstOne - 1 == 2 * mantLength) newExp++;

    long long result = newSign;
    newExp = newExp + (1ll << (expLength - 1)) - 1;

    return convertPartsToFloat(newSign, newExp, newMant, expLength, mantLength);
}

long long divisionFloatingPoint(long long a, long long b, long long expLength, long long mantLength) {
	vector<long long> value1 = parseFloatingPoint(a, expLength, mantLength), value2 = parseFloatingPoint(b, expLength, mantLength);
	
	long long maxExp = (1ll << (expLength - 1)), minExp = -(1ll << (expLength - 1)) + 1;
	long long nullMant = (1ll << mantLength);

	long long newSign = value1[0] ^ value2[0];
	long long newExp = value1[1] - value2[1];
	
	if ((value1[1] == maxExp && value1[2] != nullMant) || (value2[1] == maxExp && value2[2] != nullMant) || 
	    (value1[1] == maxExp && value1[2] == nullMant && value2[1] == maxExp && value2[2] == nullMant) ||
		(value1[1] == minExp && value1[2] == nullMant && value2[1] == minExp && value2[2] == nullMant)) {
		return (((newSign << expLength) + (1ll << expLength) - 1) << mantLength) + 1; // NAN
	}

	if ((value1[1] == maxExp && value1[2] == nullMant) || (value2[1] == minExp && value2[2] == nullMant)) {
		return ((newSign << expLength) + (1ll << expLength) - 1) << mantLength; // INF
	}

	if (value2[1] == maxExp && value2[2] == nullMant) {
		return newSign << (expLength + mantLength); // +- 0
	}

	int constToRound = 16;
	value1[2] <<= mantLength + constToRound;

    long long res = value1[2] / value2[2];

    long long firstOne = findFirstOne(res);
	newExp += firstOne - mantLength - constToRound;

	long long newMant = res ^ (1ll << firstOne);
	if (newMant == 0) return 0;

	if (firstOne > mantLength)
		newMant = rounding(newMant, firstOne - mantLength, newSign);
	else if (firstOne < mantLength) {
		newMant <<= mantLength - firstOne;
	}

    newExp = newExp + (1ll << (expLength - 1)) - 1;
    
	return convertPartsToFloat(newSign, newExp, newMant, expLength, mantLength);
}

void printFloating(long long res, long long expLength, long long mantLength) {
	vector<long long> value = parseFloatingPoint(res, expLength, mantLength);
	value[2] ^= (1ll << mantLength);

	if (value[1] == (1ll << (expLength - 1))) {
		if (value[2] == 0) {
			if (value[0]) cout << '-';
			cout << "inf" << '\n';
		} else {
			cout << "nan" << '\n';
		}
	} else { 
		if (value[0]) cout << '-';
		if (value[1] != -((1ll << (expLength - 1)) - 1) || value[2] != 0) cout << "0x1.";
		else cout << "0x0.";

        value[2] <<= (4 - mantLength % 4) % 4;
		
        int start = mantLength - mantLength % 4;
		cout << hex;
		for (int i = start ; i >= 0; i -= 4) {
			long long res = ((value[2] >> i) & (15));
			cout << ((value[2] >> i) & (15));
		}
		cout << dec;

		cout << 'p'; 

        if (value[1] >= 0) cout << '+';
        
        if (value[1] == -((1ll << (expLength - 1)) - 1) && value[2] == 0) cout << "+0" << '\n';
        else cout << value[1] << '\n';
	}
}

int solve(string&input) {
    vector < long long > data = parseString(input);
	roundingType = data[1];

	if (roundingType != 1) {
		cerr << "Wrong rounding type" << '\n';
		exit(-1);
	}

	if (data[0] != -1 && data[0] != -2) {
		data[2] = data[2] & ((1ll << (data[0] + data.back())) - 1);
		if (data.size() <= 4) printFixed(data[2], data.back(), data[0]);
		else {
			data[4] = data[4] & ((1ll << (data[0] + data.back())) - 1);
			if (data[3] == 0) {
				printFixed(sumFixed(data[2], data[4], data.back(), data[0]), data.back(), data[0]);
			} else if (data[3] == 1) {
				printFixed(subFixed(data[2], data[4], data.back(), data[0]), data.back(), data[0]);
			} else if (data[3] == 2) {
				printFixed(multiplyFixed(data[2], data[4], data.back(), data[0]), data.back(), data[0]);
			} else {
                if (data[4] == 0) {
                    cout << "error\n";
                    return 0;
                }

				printFixed(divisionFixed(data[2], data[4], data.back(), data[0]), data.back(), data[0]);
			}
		}
	} else if (data[0] == -1) {
		if (data.size() <= 4) printFloating(data[2], 5, 10);
		else {
			if (data[3] == 0) {
                printFloating(sumFloatingPoint(data[2], data[4], 5, 10), 5, 10);
			} else if (data[3] == 1) {
                data[4] ^= (1ll << 15);
				printFloating(sumFloatingPoint(data[2], data[4], 5, 10), 5, 10);
			} else if (data[3] == 2) {
                printFloating(multiplyFloatingPoint(data[2], data[4], 5, 10), 5, 10);
			} else {
				printFloating(divisionFloatingPoint(data[2], data[4], 5, 10), 5, 10);
			}
		}
	} else if (data[0] == -2) {
		if (data.size() <= 4) printFloating(data[2], 8, 23);
		else {
			if (data[3] == 0) {
                printFloating(sumFloatingPoint(data[2], data[4], 8, 23), 8, 23);
			} else if (data[3] == 1) {
				data[4] ^= (1ll << 31);
				printFloating(sumFloatingPoint(data[2], data[4], 8, 23), 8, 23);
			} else if (data[3] == 2) {
				printFloating(multiplyFloatingPoint(data[2], data[4], 8, 23), 8, 23);
			} else {
				printFloating(divisionFloatingPoint(data[2], data[4], 8, 23), 8, 23);
			}
		}
	}

    return 0;
}

int main(int argc, char* argv[]) {
	try {
		string input = "";
        // getline(cin, input);
		for (int i = 1; i < argc; i++) {
			input += argv[i]; input += ' ';
		}
		return solve(input);
	} 
	catch(std::exception&e) {
		cerr << "Something went wrong\n";
		return -1;
	}
}
