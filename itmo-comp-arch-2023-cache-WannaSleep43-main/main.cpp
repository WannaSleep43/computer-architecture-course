#include <vector>
#include <cstdio>
#include <cstdint>
using namespace std;

// some consts
const int MEM_SIZE = 1 << 20 >> 3;
const int ADDR_LEN = 20;
const int CACHE_WAY = 4;
const int CACHE_TAG_LEN = 9;
const int CACHE_IDX_LEN = 4;
const int CACHE_OFFSET_LEN = 7;
const int CACHE_SIZE = 8 * 1024;  
const int CACHE_LINE_SIZE = 128;
const int CACHE_LINE_COUNT = 64;
const int CACHE_SETS_COUNT = 16;

int lruTotal = 0, plruTotal = 0;
int lruMiss = 0, plruMiss = 0;
int ticks = 0, pticks = 0;

class MEMORY {
    private:
        vector<int> data = vector<int>(MEM_SIZE, 0);
        int ptr = 0;
    public:
        int getFree(int bytes) {
            ptr += bytes;
            return ptr - bytes;
        }
    
        int read(int address) {
            return data[address];
        }

        void write(int address, int val) {
            data[address] = val;
            return;
        }
        
};

MEMORY memory = MEMORY();

class CACHE_LINE {
    private:
        vector<int> memo = vector<int>(CACHE_LINE_SIZE);
        int tag, LRU = 0, pLRU = 0;
        int msi = 2; // 0 - Modified, 1 Shared, 2 Invalid
        int politics = 0;

        void readMemory(int prefix) {
            msi = 1;

            if (politics == 0) {
                ticks += 100 + (CACHE_LINE_SIZE / 2);
            } else {
                pticks += 100 + (CACHE_LINE_SIZE / 2);
            }

            for (int i = 0; i < CACHE_LINE_SIZE; i++) {
                memo[i] = memory.read(prefix + i);
            }
        }
    public:
        CACHE_LINE() {
            msi = 2;
        }

        CACHE_LINE(int TAG, int idx, int pol) {
            tag = TAG;
            msi = 1;
            politics = pol;
            readMemory(((tag << CACHE_IDX_LEN) + idx) << CACHE_OFFSET_LEN);
        }

        int getTag() { return tag; }

        int getMsi() { return msi; }

        int getLRU() { return LRU; }

        int getpLRU() { return pLRU; }

        void upd(int lru, int plru) {
            LRU = lru;
            setpLRU(plru);
        }

        void setpLRU(int plru) {
            pLRU = plru;
        }

        int read(int offset, int bytes, int lru, int plru) {
            upd(lru, plru);
            int res = 0;
            for (int i = offset + bytes - 1; i >= offset; i--) {
                res = (res << 8) + memo[i];
            }
            return res;
        }

        void write(int offset, int bytes, int val, int lru, int plru) {
            upd(lru, plru);
            for (int i = offset + bytes - 1; i >= offset; i--) {
                memo[i] = val & ((1 << 8) - 1);
                val >>= 8;
            }
            msi = 0;
        }

        void writeMemory(int prefix) {
            if (msi != 0) return;
            if (politics == 0) {
                ticks += 100 + (CACHE_LINE_SIZE / 2) + 1;
            } else {
                pticks += 100 + (CACHE_LINE_SIZE / 2) + 1;
            }

            for (int i = 0; i < CACHE_LINE_SIZE; i++) {
                memory.write(memo[i], prefix + i);
            }
        }
};

class BLOCK {
    private:
        vector<CACHE_LINE> lines = vector<CACHE_LINE>(CACHE_WAY);
        int index = 0, lru = 0, plru = 0;
        bool politics = false; // LRU(false) or pLRU(true)
    public:
        BLOCK(int ind, int pol) {
            index = ind;
            politics = pol;
        }

        int get_index() {
            return index;
        }

        int updplru(int cur) {
            if (cur == 1) return 1;
            plru = (plru + 1) % CACHE_WAY;
            if (plru == 0) {
                for (int i = 0; i < CACHE_WAY; i++) {
                    lines[i].setpLRU(0);
                }
                plru = 1;
            }
            return 1;
        }

        int findLeastRecently() {
            int minLRU = 0;
            int minpLRU = 0;
            for (int i = 0; i < CACHE_WAY; i++) {
                if (lines[i].getMsi() == 2) return i;
                if (lines[i].getLRU() < lines[minLRU].getLRU()) minLRU = i;
                if (lines[i].getpLRU() < lines[minpLRU].getpLRU()) minpLRU = i;
            }

            return (politics) ? minpLRU : minLRU;
        }

        void miss() {
            if (politics == 0) {
                ticks += 4;
                ticks++;
            } else {
                pticks += 4;
                pticks++;
            }
            if (politics == 0) lruMiss++;
            else plruMiss++;
        }

        void hit() {
            if (politics == 0) {
                ticks += 6;
                ticks++;
            } else {
                pticks += 6;
                pticks++;
            }
        }

        int read(int address, int bytes) {
            int tag = (address >> (CACHE_OFFSET_LEN + CACHE_IDX_LEN));
            for (int i = 0; i < CACHE_WAY; i++) {
                if (lines[i].getTag() == tag && lines[i].getMsi() != 2) {
                    hit();
                    return lines[i].read(address & ((1 << CACHE_OFFSET_LEN) - 1), bytes, lru++, updplru(lines[i].getpLRU()));
                }
            }

            miss();

            int change = findLeastRecently();
            lines[change].writeMemory(((tag << CACHE_IDX_LEN) + index) << CACHE_OFFSET_LEN);
            plru -= lines[change].getpLRU();
            lines[change] = CACHE_LINE(tag, index, politics);
            return lines[change].read(address & ((1 << CACHE_OFFSET_LEN) - 1), bytes, lru++, updplru(lines[change].getpLRU()));
        }

        void write(int address, int value, int bytes) {
            int tag = (address >> (CACHE_OFFSET_LEN + CACHE_IDX_LEN));
            for (int i = 0; i < CACHE_WAY; i++) {
                if (lines[i].getTag() == tag && lines[i].getMsi() != 2) {
                    hit();
                    return lines[i].write(address & ((1 << CACHE_OFFSET_LEN) - 1), value, bytes, lru++, updplru(lines[i].getpLRU()));
                }
            }

            miss();

            int change = findLeastRecently();
            lines[change].writeMemory(((tag << CACHE_IDX_LEN) + index) << CACHE_OFFSET_LEN);
            plru -= lines[change].getpLRU();
            lines[change] = CACHE_LINE(tag, index, politics);
            return lines[change].write(address & ((1 << CACHE_OFFSET_LEN) - 1), value, bytes, lru++, updplru(lines[change].getpLRU()));
        }
};

class CACHE {
    private:
        int pol = 0;
        vector<BLOCK> blocks = vector<BLOCK>();
    public:
        CACHE(int politics) {
            pol = politics;
            for (int i = 0; i < CACHE_SETS_COUNT; i++) blocks.push_back(BLOCK(i, politics));
        }

        int read(int address, int bytes) {
            if (pol == 0) {
                lruTotal++;
                ticks += (bytes + 1) / 2;
            } else {
                plruTotal++;
                pticks += (bytes + 1) / 2;
            }
            int index = (address >> CACHE_OFFSET_LEN) & ((1 << CACHE_IDX_LEN) - 1);
            return blocks[index].read(address, bytes);
        }

        void write(int address, int val, int bytes) {
            if (pol == 0) {
                lruTotal++;
                ticks += (bytes + 1) / 2 + 1;
            } else {
                plruTotal++;
                pticks += (bytes + 1) / 2 + 1;
            }
            int index = (address >> CACHE_OFFSET_LEN) & ((1 << CACHE_IDX_LEN) - 1);
            return blocks[index].write(address, val, bytes);
        }
};

vector<vector<int>> init(int n, int m, int bytes) {
    vector<vector<int>> address = vector<vector<int>>(n, vector<int>(m, 0));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            address[i][j] = memory.getFree(bytes);
        }
    }
    return address;
}

#define M 64
#define N 60
#define K 32
int8_t a[M][K];
int16_t b[K][N];
int32_t c[M][N];


void mmul()
{
    vector<vector<int>> aAddress = init(M, K, 1);
    vector<vector<int>> bAddress = init(K, N, 2);
    vector<vector<int>> cAddress = init(M, N, 4);

    int8_t *pa = *a;
    int32_t *pc = *c;
    
    CACHE cache = CACHE(0); // lru
    CACHE pcache = CACHE(1); // plru

    ticks += 1; // initial
    pticks += 1;
    for (int y = 0; y < M; y++)
    {
        ticks += 2; // new iteration, initial
        pticks += 2;

        for (int x = 0; x < N; x++)
        {
            ticks += 1; // new iteration
            pticks += 1;
            int16_t *pb = *b;
            
            ticks += 1; // init s
            pticks += 1;
            int32_t s = 0;

            ticks += 1; // initial
            pticks += 1;
            for (int k = 0; k < K; k++)
            {
                ticks += 1; // new iteration
                pticks += 1;

                ticks += 5 + 3; // mul, read pa[k], read pb[x], +
                pticks += 8;
                s += pa[k] * pb[x];
                cache.read(aAddress[y][k], 1);
                cache.read(bAddress[k][x], 2);
                pcache.read(aAddress[y][k], 1);
                pcache.read(bAddress[k][x], 2);
                
                pb += N;
                ticks += 1; // read pb, +
                pticks += 1;
            }
            pc[x] = s;
            ticks += 1; // read s
            pticks += 1;

            cache.write(cAddress[y][x], s, 4);
            pcache.write(cAddress[y][x], s, 4);
        }
        pa += K;
        ticks += 1; // read pa, +
        pticks += 1;
        pc += N;
        ticks += 1; // read pc, +
        pticks += 1;
    }

    // выход из функции
    ticks += 1;
    pticks += 1;
}


int main() {
    mmul();

    double lruANS = (lruTotal - lruMiss) * 100 / (float) lruTotal;
    double plruANS = (plruTotal - plruMiss) * 100 / (float) plruTotal;

    printf("LRU:\thit perc. %3.4f%%\ttime: %d\npLRU:\thit perc. %3.4f%%\ttime: %d\n", lruANS, ticks, plruANS, pticks);

    return 0;
}
