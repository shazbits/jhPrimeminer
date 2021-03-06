// Copyright (c) 2013 Primecoin developers
// Distributed under conditional MIT/X11 software license,
// see the accompanying file COPYING

#include "global.h"

// Prime Table
//std::vector<unsigned int> vPrimes;
uint32* vPrimes;
uint32 vPrimesSize = 0;


// changed to return the ticks since reboot
// doesnt need to be the correct time, just a more or less random input value
uint64 GetTimeMicros()
{
	LARGE_INTEGER t;
	QueryPerformanceCounter(&t);
	return (uint64)t.QuadPart;
}

//void GeneratePrimeTable()
//{
//	static const unsigned int nPrimeTableLimit = nMaxSieveSize;
//	vPrimes.clear();
//	// Generate prime table using sieve of Eratosthenes
//	std::vector<bool> vfComposite (nPrimeTableLimit, false);
//	for (unsigned int nFactor = 2; nFactor * nFactor < nPrimeTableLimit; nFactor++)
//	{
//		if (vfComposite[nFactor])
//			continue;
//		for (unsigned int nComposite = nFactor * nFactor; nComposite < nPrimeTableLimit; nComposite += nFactor)
//			vfComposite[nComposite] = true;
//	}
//	for (unsigned int n = 2; n < nPrimeTableLimit; n++)
//		if (!vfComposite[n])
//			vPrimes.push_back(n);
//	printf("GeneratePrimeTable() : prime table [1, %d] generated with %lu primes", nPrimeTableLimit, vPrimes.size());
//	//BOOST_FOREACH(unsigned int nPrime, vPrimes)
//	//    printf(" %u", nPrime);
//	printf("\n");
//}

void GeneratePrimeTable()
{
	static const unsigned int nPrimeTableLimit = nMaxSieveSize;
	//vPrimes.clear();
	vPrimesSize = 0;
	vPrimes = (uint32*)malloc(sizeof(uint32)*nPrimeTableLimit);
	// Generate prime table using sieve of Eratosthenes
	//std::vector<bool> vfComposite (nPrimeTableLimit, false);
	uint8* vfComposite = (uint8*)malloc(nPrimeTableLimit);
	RtlZeroMemory(vfComposite, nPrimeTableLimit);
	for (unsigned int nFactor = 2; nFactor * nFactor < nPrimeTableLimit; nFactor++)
	{
		if (vfComposite[nFactor])
			continue;
		for (unsigned int nComposite = nFactor * nFactor; nComposite < nPrimeTableLimit; nComposite += nFactor)
			vfComposite[nComposite] = true;
	}
	for (unsigned int n = 2; n < nPrimeTableLimit; n++)
		if (!vfComposite[n])
		{
			vPrimes[vPrimesSize] = n;
			vPrimesSize++;
			//customBuffer_add(list_primeTable, &n);//vPrimes.push_back(n);
		}
	printf("GeneratePrimeTable() : prime table [1, %d] generated with %lu primes", nPrimeTableLimit, vPrimesSize);
	//BOOST_FOREACH(unsigned int nPrime, vPrimes)
	//    printf(" %u", nPrime);
	printf("\n");
	free(vfComposite);
	vPrimes = (uint32*)realloc(vPrimes, sizeof(uint32)*vPrimesSize);
}

//// Get next prime number of p
//bool PrimeTableGetNextPrime(unsigned int& p)
//{
//	//BOOST_FOREACH(unsigned int nPrime, vPrimes)
//	for(uint32 i=0; i<vPrimes.size(); i++)
//	{
//		unsigned int nPrime = vPrimes[i];
//		if (nPrime > p)
//		{
//			p = nPrime;
//			return true;
//		}
//	}
//	return false;
//}
//

//// Get previous prime number of p
//bool PrimeTableGetPreviousPrime(unsigned int& p)
//{
//	unsigned int nPrevPrime = 0;
//	//BOOST_FOREACH(unsigned int nPrime, vPrimes)
//	for(uint32 i=0; i<vPrimes.size(); i++)
//	{
//		unsigned int nPrime = vPrimes[i];
//		if (nPrime >= p)
//			break;
//		nPrevPrime = nPrime;
//	}
//	if (nPrevPrime)
//	{
//		p = nPrevPrime;
//		return true;
//	}
//	return false;
//}

// Get next prime number of p
bool PrimeTableGetNextPrime(unsigned int* p)
{
	for(uint32 i=0; i<vPrimesSize; i++)
	{
		if (vPrimes[i] > *p)
		{
			*p = vPrimes[i];
			return true;
		}
	}
	return false;
}

// Get previous prime number of p
bool PrimeTableGetPreviousPrime(unsigned int* p)
{
	uint32 nPrevPrime = 0;
	for(uint32 i=0; i<vPrimesSize; i++)
	{
		if (vPrimes[i] >= *p)
			break;
		nPrevPrime = vPrimes[i];
	}
	if (nPrevPrime)
	{
		*p = nPrevPrime;
		return true;
	}
	return false;
}

// Compute Primorial number p#
void Primorial(unsigned int p, CBigNum& bnPrimorial)
{
	bnPrimorial = 1;
	//BOOST_FOREACH(unsigned int nPrime, vPrimes)
	//for(uint32 i=0; i<vPrimes.size(); i++)
	for(uint32 i=0; i<vPrimesSize; i++)
	{
		unsigned int nPrime = vPrimes[i];
		if (nPrime > p) break;
		bnPrimorial *= nPrime;
	}
}

// Compute first primorial number greater than or equal to pn
void PrimorialAt(CBigNum& bn, CBigNum& bnPrimorial)
{
	bnPrimorial = 1;
	//BOOST_FOREACH(unsigned int nPrime, vPrimes)
	//for(uint32 i=0; i<vPrimes.size(); i++)
	for(uint32 i=0; i<vPrimesSize; i++)
	{
		unsigned int nPrime = vPrimes[i];
		bnPrimorial *= nPrime;
		if (bnPrimorial >= bn)
			return;
	}
}

// Check Fermat probable primality test (2-PRP): 2 ** (n-1) = 1 (mod n)
// true: n is probable prime
// false: n is composite; set fractional length in the nLength output
static bool FermatProbablePrimalityTest(const CBigNum& n, unsigned int& nLength)
{
	CAutoBN_CTX pctx;
	CBigNum a = 2; // base; Fermat witness
	CBigNum e = n - 1;
	CBigNum r;
	BN_mod_exp(&r, &a, &e, &n, pctx);
	if (r == 1)
		return true;
	// Failed Fermat test, calculate fractional length
	unsigned int nFractionalLength = (((n-r) << nFractionalBits) / n).getuint();
	if (nFractionalLength >= (1 << nFractionalBits))
		return error("FermatProbablePrimalityTest() : fractional assert");
	nLength = (nLength & TARGET_LENGTH_MASK) | nFractionalLength;
	return false;
}

// Test probable primality of n = 2p +/- 1 based on Euler, Lagrange and Lifchitz
// fSophieGermain:
//   true:  n = 2p+1, p prime, aka Cunningham Chain of first kind
//   false: n = 2p-1, p prime, aka Cunningham Chain of second kind
// Return values
//   true: n is probable prime
//   false: n is composite; set fractional length in the nLength output
static bool EulerLagrangeLifchitzPrimalityTest(const CBigNum& n, bool fSophieGermain, unsigned int& nLength)
{
	CAutoBN_CTX pctx;
	CBigNum a = 2;
	CBigNum e = (n - 1) >> 1;
	CBigNum r;
	BN_mod_exp(&r, &a, &e, &n, pctx);
	CBigNum nMod8 = n % 8;
	bool fPassedTest = false;
	if (fSophieGermain && (nMod8 == 7)) // Euler & Lagrange
		fPassedTest = (r == 1);
	else if (fSophieGermain && (nMod8 == 3)) // Lifchitz
		fPassedTest = ((r+1) == n);
	else if ((!fSophieGermain) && (nMod8 == 5)) // Lifchitz
		fPassedTest = ((r+1) == n);
	else if ((!fSophieGermain) && (nMod8 == 1)) // LifChitz
		fPassedTest = (r == 1);
	else
		return error("EulerLagrangeLifchitzPrimalityTest() : invalid n %% 8 = %d, %s", nMod8.getint(), (fSophieGermain? "first kind" : "second kind"));

	if (fPassedTest)
		return true;
	// Failed test, calculate fractional length
	r = (r * r) % n; // derive Fermat test remainder
	unsigned int nFractionalLength = (((n-r) << nFractionalBits) / n).getuint();
	if (nFractionalLength >= (1 << nFractionalBits))
		return error("EulerLagrangeLifchitzPrimalityTest() : fractional assert");
	nLength = (nLength & TARGET_LENGTH_MASK) | nFractionalLength;
	return false;
}

// Proof-of-work Target (prime chain target):
//   format - 32 bit, 8 length bits, 24 fractional length bits

unsigned int nTargetInitialLength = 7; // initial chain length target
unsigned int nTargetMinLength = 6;     // minimum chain length target

unsigned int TargetGetLimit()
{
	return (nTargetMinLength << nFractionalBits);
}

unsigned int TargetGetInitial()
{
	return (nTargetInitialLength << nFractionalBits);
}

unsigned int TargetGetLength(unsigned int nBits)
{
	return ((nBits & TARGET_LENGTH_MASK) >> nFractionalBits);
}

bool TargetSetLength(unsigned int nLength, unsigned int& nBits)
{
	if (nLength >= 0xff)
		return error("TargetSetLength() : invalid length=%u", nLength);
	nBits &= TARGET_FRACTIONAL_MASK;
	nBits |= (nLength << nFractionalBits);
	return true;
}

void TargetIncrementLength(unsigned int& nBits)
{
	nBits += (1 << nFractionalBits);
}

void TargetDecrementLength(unsigned int& nBits)
{
	if (TargetGetLength(nBits) > nTargetMinLength)
		nBits -= (1 << nFractionalBits);
}

unsigned int TargetGetFractional(unsigned int nBits)
{
	return (nBits & TARGET_FRACTIONAL_MASK);
}

uint64 TargetGetFractionalDifficulty(unsigned int nBits)
{
	return (nFractionalDifficultyMax / (uint64) ((1ull<<nFractionalBits) - TargetGetFractional(nBits)));
}

bool TargetSetFractionalDifficulty(uint64 nFractionalDifficulty, unsigned int& nBits)
{
	if (nFractionalDifficulty < nFractionalDifficultyMin)
		return error("TargetSetFractionalDifficulty() : difficulty below min");
	uint64 nFractional = nFractionalDifficultyMax / nFractionalDifficulty;
	if (nFractional > (1u<<nFractionalBits))
		return error("TargetSetFractionalDifficulty() : fractional overflow: nFractionalDifficulty=%016I64d", nFractionalDifficulty);
	nFractional = (1u<<nFractionalBits) - nFractional;
	nBits &= TARGET_LENGTH_MASK;
	nBits |= (unsigned int)nFractional;
	return true;
}

std::string TargetToString(unsigned int nBits)
{
	__debugbreak(); // return strprintf("%02x.%06x", TargetGetLength(nBits), TargetGetFractional(nBits));
	return NULL; // todo
}

unsigned int TargetFromInt(unsigned int nLength)
{
	return (nLength << nFractionalBits);
}

// Get mint value from target
// Primecoin mint rate is determined by target
//   mint = 999 / (target length ** 2)
// Inflation is controlled via Moore's Law
bool TargetGetMint(unsigned int nBits, uint64& nMint)
{
	nMint = 0;
	static uint64 nMintLimit = 999ull * COIN;
	CBigNum bnMint = nMintLimit;
	if (TargetGetLength(nBits) < nTargetMinLength)
		return error("TargetGetMint() : length below minimum required, nBits=%08x", nBits);
	bnMint = (bnMint << nFractionalBits) / nBits;
	bnMint = (bnMint << nFractionalBits) / nBits;
	bnMint = (bnMint / CENT) * CENT;  // mint value rounded to cent
	nMint = bnMint.getuint256().Get64();
	if (nMint > nMintLimit)
	{
		nMint = 0;
		return error("TargetGetMint() : mint value over limit, nBits=%08x", nBits);
	}
	return true;
}

// Get next target value
bool TargetGetNext(unsigned int nBits, int64 nInterval, int64 nTargetSpacing, int64 nActualSpacing, unsigned int& nBitsNext)
{
	nBitsNext = nBits;
	// Convert length into fractional difficulty
	uint64 nFractionalDifficulty = TargetGetFractionalDifficulty(nBits);
	// Compute new difficulty via exponential moving toward target spacing
	CBigNum bnFractionalDifficulty = nFractionalDifficulty;
	bnFractionalDifficulty *= ((nInterval + 1) * nTargetSpacing);
	bnFractionalDifficulty /= ((nInterval - 1) * nTargetSpacing + nActualSpacing + nActualSpacing);
	if (bnFractionalDifficulty > nFractionalDifficultyMax)
		bnFractionalDifficulty = nFractionalDifficultyMax;
	if (bnFractionalDifficulty < nFractionalDifficultyMin)
		bnFractionalDifficulty = nFractionalDifficultyMin;
	uint64 nFractionalDifficultyNew = bnFractionalDifficulty.getuint256().Get64();
	//if (fDebug && GetBoolArg("-printtarget"))
	//	printf("TargetGetNext() : nActualSpacing=%d nFractionDiff=%016"PRI64x" nFractionDiffNew=%016"PRI64x"\n", (int)nActualSpacing, nFractionalDifficulty, nFractionalDifficultyNew);
	// Step up length if fractional past threshold
	if (nFractionalDifficultyNew > nFractionalDifficultyThreshold)
	{
		nFractionalDifficultyNew = nFractionalDifficultyMin;
		TargetIncrementLength(nBitsNext);
	}
	// Step down length if fractional at minimum
	else if (nFractionalDifficultyNew == nFractionalDifficultyMin && TargetGetLength(nBitsNext) > nTargetMinLength)
	{
		nFractionalDifficultyNew = nFractionalDifficultyThreshold;
		TargetDecrementLength(nBitsNext);
	}
	// Convert fractional difficulty back to length
	if (!TargetSetFractionalDifficulty(nFractionalDifficultyNew, nBitsNext))
		return error("TargetGetNext() : unable to set fractional difficulty prev=0x%016I64d new=0x%016I64d", nFractionalDifficulty, nFractionalDifficultyNew);
	return true;
}

// Test Probable Cunningham Chain for: n
// fSophieGermain:
//   true - Test for Cunningham Chain of first kind (n, 2n+1, 4n+3, ...)
//   false - Test for Cunningham Chain of second kind (n, 2n-1, 4n-3, ...)
// Return value:
//   true - Probable Cunningham Chain found (length at least 2)
//   false - Not Cunningham Chain
static bool ProbableCunninghamChainTest(const CBigNum& n, bool fSophieGermain, bool fFermatTest, unsigned int& nProbableChainLength)
{
	nProbableChainLength = 0;
	CBigNum N = n;

	// Fermat test for n first
	if (!FermatProbablePrimalityTest(N, nProbableChainLength))
		return false;

	// Euler-Lagrange-Lifchitz test for the following numbers in chain
	while (true)
	{
		TargetIncrementLength(nProbableChainLength);
		N = N + N + (fSophieGermain? 1 : (-1));
		if (fFermatTest)
		{
			if (!FermatProbablePrimalityTest(N, nProbableChainLength))
				break;
		}
		else
		{
			if (!EulerLagrangeLifchitzPrimalityTest(N, fSophieGermain, nProbableChainLength))
				break;
		}
	}

	return (TargetGetLength(nProbableChainLength) >= 2);
}

// Test probable prime chain for: nOrigin
// Return value:
//   true - Probable prime chain found (one of nChainLength meeting target)
//   false - prime chain too short (none of nChainLength meeting target)
bool ProbablePrimeChainTest(const CBigNum& bnPrimeChainOrigin, unsigned int nBits, bool fFermatTest, unsigned int& nChainLengthCunningham1, unsigned int& nChainLengthCunningham2, unsigned int& nChainLengthBiTwin)
{
	nChainLengthCunningham1 = 0;
	nChainLengthCunningham2 = 0;
	nChainLengthBiTwin = 0;

	// Test for Cunningham Chain of first kind
	ProbableCunninghamChainTest(bnPrimeChainOrigin-1, true, fFermatTest, nChainLengthCunningham1);
	// Test for Cunningham Chain of second kind
	ProbableCunninghamChainTest(bnPrimeChainOrigin+1, false, fFermatTest, nChainLengthCunningham2);
	// Figure out BiTwin Chain length
	// BiTwin Chain allows a single prime at the end for odd length chain
	nChainLengthBiTwin =
		(TargetGetLength(nChainLengthCunningham1) > TargetGetLength(nChainLengthCunningham2))?
		(nChainLengthCunningham2 + TargetFromInt(TargetGetLength(nChainLengthCunningham2)+1)) :
	(nChainLengthCunningham1 + TargetFromInt(TargetGetLength(nChainLengthCunningham1)));
	return (nChainLengthCunningham1 >= nBits || nChainLengthCunningham2 >= nBits || nChainLengthBiTwin >= nBits);
}

//// Sieve for mining
//boost::thread_specific_ptr<CSieveOfEratosthenes> psieve;

// Mine probable prime chain of form: n = h * p# +/- 1
bool MineProbablePrimeChain(CSieveOfEratosthenes** psieve, primecoinBlock_t* block, CBigNum& bnFixedMultiplier, bool& fNewBlock, unsigned int& nTriedMultiplier, unsigned int& nProbableChainLength, unsigned int& nTests, unsigned int& nPrimesHit)
{
	nProbableChainLength = 0;
	nTests = 0;
	nPrimesHit = 0;
	if (fNewBlock && *psieve != NULL)
	{
		// Must rebuild the sieve
		delete *psieve;
		*psieve = NULL;
	}
	fNewBlock = false;

	//int64 nStart, nCurrent; // microsecond timer
	if (*psieve == NULL)
	{
		// Build sieve
		uint32 nStart = GetTickCount();
		//uint32 sieveStopTimeLimit = nStart + 2000;
		//for(uint32 i=0; i<256/8; i++)
		//	printf("%02x", block->blockHeaderHash.begin()[i]);
		*psieve = new CSieveOfEratosthenes(nMaxSieveSize, block->nBits, block->blockHeaderHash, bnFixedMultiplier);
		(*psieve)->WeaveFastAll();
		
		//while ((*psieve)->WeaveOriginal() && GetTickCount() < sieveStopTimeLimit );
		//printf("MineProbablePrimeChain() : new sieve (%u/%u) ready in %ums multiplier: %u\n", (*psieve)->GetCandidateCount(), nMaxSieveSize, (unsigned int) (GetTickCount() - nStart), bnFixedMultiplier);
	}

	CBigNum bnChainOrigin;

	//nStart = GetTickCount();
	//nCurrent = nStart;

	//uint32 timeStop = GetTickCount() + 25000;
	uint32 nTries = 0;
	while ( nTries < 15000 && block->serverData.blockHeight == jhMiner_getCurrentWorkBlockHeight() )
	{
		nTries++;
		nTests++;
		if (!(*psieve)->GetNextCandidateMultiplier(nTriedMultiplier))
		{
			// power tests completed for the sieve
			delete *psieve;
			*psieve = NULL;
			fNewBlock = true; // notify caller to change nonce
			return false;
		}
		bnChainOrigin = CBigNum(block->blockHeaderHash) * bnFixedMultiplier * nTriedMultiplier;
		unsigned int nChainLengthCunningham1 = 0;
		unsigned int nChainLengthCunningham2 = 0;
		unsigned int nChainLengthBiTwin = 0;
		bool canSubmitAsShare = ProbablePrimeChainTest(bnChainOrigin, block->nBits, false, nChainLengthCunningham1, nChainLengthCunningham2, nChainLengthBiTwin);
		nProbableChainLength = max(max(nChainLengthCunningham1, nChainLengthCunningham2), nChainLengthBiTwin);
		if( nProbableChainLength >= block->serverData.nBitsForShare )
			canSubmitAsShare = true;
		//if( nBitsGen >= 0x03000000 )
		//	printf("%08X\n", nBitsGen);
		primeStats.primeChainsFound++;
		//if( nProbableChainLength > 0x03000000 )
		//	primeStats.qualityPrimesFound++;
		if( nProbableChainLength > primeStats.bestPrimeChainDifficulty )
			primeStats.bestPrimeChainDifficulty = nProbableChainLength;
		if(canSubmitAsShare)
		{
			block->bnPrimeChainMultiplier = bnFixedMultiplier * nTriedMultiplier;
			// update server data
			block->serverData.client_shareBits = nProbableChainLength;
			// generate block raw data
			uint8 blockRawData[256] = {0};
			memcpy(blockRawData, block, 80);
			uint32 writeIndex = 80;
			sint32 lengthBN = 0;
			std::vector<unsigned char> bnSerializeData = block->bnPrimeChainMultiplier.getvch();
			lengthBN = bnSerializeData.size();
			*(uint8*)(blockRawData+writeIndex) = (uint8)lengthBN; // varInt (we assume it always has a size low enough for 1 byte)
			writeIndex += 1;
			memcpy(blockRawData+writeIndex, &bnSerializeData[0], lengthBN);
			writeIndex += lengthBN;	
			// switch endianness
			for(uint32 f=0; f<256/4; f++)
			{
				*(uint32*)(blockRawData+f*4) = _swapEndianessU32(*(uint32*)(blockRawData+f*4));
			}
			// submit this share
			jhMiner_pushShare_primecoin(blockRawData, block);
			//printf("Probable prime chain found for block=%s!!\n  Target: %s\n  Length: (%s %s %s)\n", block.GetHash().GetHex().c_str(),TargetToString(block.nBits).c_str(), TargetToString(nChainLengthCunningham1).c_str(), TargetToString(nChainLengthCunningham2).c_str(), TargetToString(nChainLengthBiTwin).c_str());
			nProbableChainLength = max(max(nChainLengthCunningham1, nChainLengthCunningham2), nChainLengthBiTwin);
			// since we are using C structs here we have to make sure the memory for the CBigNum in the block is freed again
			//delete *psieve;
			//*psieve = NULL;
			//block->bnPrimeChainMultiplier = NULL;
			RtlZeroMemory(blockRawData, 256);
			delete *psieve;
			*psieve = NULL;
			// dont quit if we find a share, there could be other shares in the remaining prime candidates
			return false;
		}
		//if(TargetGetLength(nProbableChainLength) >= 1)
		//	nPrimesHit++;
		//nCurrent = GetTickCount();
	}
	if( *psieve )
	{
		delete *psieve;
		*psieve = NULL;
	}
	return false; // stop as timed out
}

//// Check prime proof-of-work
//bool CheckPrimeProofOfWork(uint256 hashBlockHeader, unsigned int nBits, const CBigNum& bnPrimeChainMultiplier, unsigned int& nChainType, unsigned int& nChainLength)
//{
//	// Check target
//	if (TargetGetLength(nBits) < nTargetMinLength || TargetGetLength(nBits) > 99)
//		return error("CheckPrimeProofOfWork() : invalid chain length target %s", TargetToString(nBits).c_str());
//
//	// Check header hash limit
//	if (hashBlockHeader < hashBlockHeaderLimit)
//		return error("CheckPrimeProofOfWork() : block header hash under limit");
//	// Check target for prime proof-of-work
//	CBigNum bnPrimeChainOrigin = CBigNum(hashBlockHeader) * bnPrimeChainMultiplier;
//	if (bnPrimeChainOrigin < bnPrimeMin)
//		return error("CheckPrimeProofOfWork() : prime too small");
//	// First prime in chain must not exceed cap
//	if (bnPrimeChainOrigin > bnPrimeMax)
//		return error("CheckPrimeProofOfWork() : prime too big");
//
//	// Check prime chain
//	unsigned int nChainLengthCunningham1 = 0;
//	unsigned int nChainLengthCunningham2 = 0;
//	unsigned int nChainLengthBiTwin = 0;
//	if (!ProbablePrimeChainTest(bnPrimeChainOrigin, nBits, false, nChainLengthCunningham1, nChainLengthCunningham2, nChainLengthBiTwin))
//		return error("CheckPrimeProofOfWork() : failed prime chain test target=%s length=(%s %s %s)", TargetToString(nBits).c_str(),
//		TargetToString(nChainLengthCunningham1).c_str(), TargetToString(nChainLengthCunningham2).c_str(), TargetToString(nChainLengthBiTwin).c_str());
//	if (nChainLengthCunningham1 < nBits && nChainLengthCunningham2 < nBits && nChainLengthBiTwin < nBits)
//		return error("CheckPrimeProofOfWork() : prime chain length assert target=%s length=(%s %s %s)", TargetToString(nBits).c_str(),
//		TargetToString(nChainLengthCunningham1).c_str(), TargetToString(nChainLengthCunningham2).c_str(), TargetToString(nChainLengthBiTwin).c_str());
//
//	// Double check prime chain with Fermat tests only
//	unsigned int nChainLengthCunningham1FermatTest = 0;
//	unsigned int nChainLengthCunningham2FermatTest = 0;
//	unsigned int nChainLengthBiTwinFermatTest = 0;
//	if (!ProbablePrimeChainTest(bnPrimeChainOrigin, nBits, true, nChainLengthCunningham1FermatTest, nChainLengthCunningham2FermatTest, nChainLengthBiTwinFermatTest))
//		return error("CheckPrimeProofOfWork() : failed Fermat test target=%s length=(%s %s %s) lengthFermat=(%s %s %s)", TargetToString(nBits).c_str(),
//		TargetToString(nChainLengthCunningham1).c_str(), TargetToString(nChainLengthCunningham2).c_str(), TargetToString(nChainLengthBiTwin).c_str(),
//		TargetToString(nChainLengthCunningham1FermatTest).c_str(), TargetToString(nChainLengthCunningham2FermatTest).c_str(), TargetToString(nChainLengthBiTwinFermatTest).c_str());
//	if (nChainLengthCunningham1 != nChainLengthCunningham1FermatTest ||
//		nChainLengthCunningham2 != nChainLengthCunningham2FermatTest ||
//		nChainLengthBiTwin != nChainLengthBiTwinFermatTest)
//		return error("CheckPrimeProofOfWork() : failed Fermat-only double check target=%s length=(%s %s %s) lengthFermat=(%s %s %s)", TargetToString(nBits).c_str(), 
//		TargetToString(nChainLengthCunningham1).c_str(), TargetToString(nChainLengthCunningham2).c_str(), TargetToString(nChainLengthBiTwin).c_str(),
//		TargetToString(nChainLengthCunningham1FermatTest).c_str(), TargetToString(nChainLengthCunningham2FermatTest).c_str(), TargetToString(nChainLengthBiTwinFermatTest).c_str());
//
//	// Select the longest primechain from the three chain types
//	nChainLength = nChainLengthCunningham1;
//	nChainType = PRIME_CHAIN_CUNNINGHAM1;
//	if (nChainLengthCunningham2 > nChainLength)
//	{
//		nChainLength = nChainLengthCunningham2;
//		nChainType = PRIME_CHAIN_CUNNINGHAM2;
//	}
//	if (nChainLengthBiTwin > nChainLength)
//	{
//		nChainLength = nChainLengthBiTwin;
//		nChainType = PRIME_CHAIN_BI_TWIN;
//	}
//
//	// Check that the certificate (bnPrimeChainMultiplier) is normalized
//	if (bnPrimeChainMultiplier % 2 == 0 && bnPrimeChainOrigin % 4 == 0)
//	{
//		unsigned int nChainLengthCunningham1Extended = 0;
//		unsigned int nChainLengthCunningham2Extended = 0;
//		unsigned int nChainLengthBiTwinExtended = 0;
//		if (ProbablePrimeChainTest(bnPrimeChainOrigin / 2, nBits, false, nChainLengthCunningham1Extended, nChainLengthCunningham2Extended, nChainLengthBiTwinExtended))
//		{ // try extending down the primechain with a halved multiplier
//			if (nChainLengthCunningham1Extended > nChainLength || nChainLengthCunningham2Extended > nChainLength || nChainLengthBiTwinExtended > nChainLength)
//				return error("CheckPrimeProofOfWork() : prime certificate not normalzied target=%s length=(%s %s %s) extend=(%s %s %s)",
//				TargetToString(nBits).c_str(),
//				TargetToString(nChainLengthCunningham1).c_str(), TargetToString(nChainLengthCunningham2).c_str(), TargetToString(nChainLengthBiTwin).c_str(),
//				TargetToString(nChainLengthCunningham1Extended).c_str(), TargetToString(nChainLengthCunningham2Extended).c_str(), TargetToString(nChainLengthBiTwinExtended).c_str());
//		}
//	}
//
//	return true;
//}

// prime target difficulty value for visualization
double GetPrimeDifficulty(unsigned int nBits)
{
	return ((double) nBits / (double) (1 << nFractionalBits));
}

// Estimate work transition target to longer prime chain
unsigned int EstimateWorkTransition(unsigned int nPrevWorkTransition, unsigned int nBits, unsigned int nChainLength)
{
	int64 nInterval = 500;
	int64 nWorkTransition = nPrevWorkTransition;
	unsigned int nBitsCeiling = 0;
	TargetSetLength(TargetGetLength(nBits)+1, nBitsCeiling);
	unsigned int nBitsFloor = 0;
	TargetSetLength(TargetGetLength(nBits), nBitsFloor);
	uint64 nFractionalDifficulty = TargetGetFractionalDifficulty(nBits);
	bool fLonger = (TargetGetLength(nChainLength) > TargetGetLength(nBits));
	if (fLonger)
		nWorkTransition = (nWorkTransition * (((nInterval - 1) * nFractionalDifficulty) >> 32) + 2 * ((uint64) nBitsFloor)) / ((((nInterval - 1) * nFractionalDifficulty) >> 32) + 2);
	else
		nWorkTransition = ((nInterval - 1) * nWorkTransition + 2 * ((uint64) nBitsCeiling)) / (nInterval + 1);
	return nWorkTransition;
}

//// prime chain type and length value
//std::string GetPrimeChainName(unsigned int nChainType, unsigned int nChainLength)
//{
//	return strprintf("%s%s", (nChainType==PRIME_CHAIN_CUNNINGHAM1)? "1CC" : ((nChainType==PRIME_CHAIN_CUNNINGHAM2)? "2CC" : "TWN"), TargetToString(nChainLength).c_str());
//}

// Weave sieve for the next prime in table
// Return values:
//   True  - weaved another prime; nComposite - number of composites removed
//   False - sieve already completed
bool CSieveOfEratosthenes::WeaveOriginal()
{
	if (nPrimeSeq >= vPrimesSize || vPrimes[nPrimeSeq] >= nSieveSize)
		return false;  // sieve has been completed
	CBigNum p = vPrimes[nPrimeSeq];
	if (bnFixedFactor % p == 0)
	{
		// Nothing in the sieve is divisible by this prime
		nPrimeSeq++;
		return true;
	}
	// Find the modulo inverse of fixed factor
	CBigNum bnFixedInverse;
	if (!BN_mod_inverse(&bnFixedInverse, &bnFixedFactor, &p, pctx))
		return error("CSieveOfEratosthenes::Weave(): BN_mod_inverse of fixed factor failed for prime #%u=%u", nPrimeSeq, vPrimes[nPrimeSeq]);
	CBigNum bnTwo = 2;
	CBigNum bnTwoInverse;
	if (!BN_mod_inverse(&bnTwoInverse, &bnTwo, &p, pctx))
		return error("CSieveOfEratosthenes::Weave(): BN_mod_inverse of 2 failed for prime #%u=%u", nPrimeSeq, vPrimes[nPrimeSeq]);

	// Weave the sieve for the prime
	unsigned int nChainLength = TargetGetLength(nBits);
	for (unsigned int nBiTwinSeq = 0; nBiTwinSeq < 2 * nChainLength; nBiTwinSeq++)
	{
		// Find the first number that's divisible by this prime
		int nDelta = ((nBiTwinSeq % 2 == 0)? (-1) : 1);
		unsigned int nSolvedMultiplier = ((bnFixedInverse * (p - nDelta)) % p).getuint();
		if (nBiTwinSeq % 2 == 1)
			bnFixedInverse *= bnTwoInverse; // for next number in chain

		unsigned int nPrime = vPrimes[nPrimeSeq];
		if (nBiTwinSeq < nChainLength)
			for (unsigned int nVariableMultiplier = nSolvedMultiplier; nVariableMultiplier < nSieveSize; nVariableMultiplier += nPrime)
				vfCompositeBiTwin[nVariableMultiplier>>3] |= 1<<(nVariableMultiplier&7);
		if (((nBiTwinSeq & 1u) == 0))
			for (unsigned int nVariableMultiplier = nSolvedMultiplier; nVariableMultiplier < nSieveSize; nVariableMultiplier += nPrime)
				vfCompositeCunningham1[nVariableMultiplier>>3] |= 1<<(nVariableMultiplier&7);
		if (((nBiTwinSeq & 1u) == 1u))
			for (unsigned int nVariableMultiplier = nSolvedMultiplier; nVariableMultiplier < nSieveSize; nVariableMultiplier += nPrime)
				vfCompositeCunningham2[nVariableMultiplier>>3] |= 1<<(nVariableMultiplier&7);
	}
	nPrimeSeq++;
	//delete[] p;
	return true;
}

//bool CSieveOfEratosthenes::WeaveFast()
//{
//	if (nPrimeSeq >= vPrimesSize || vPrimes[nPrimeSeq] >= nSieveSize)
//		return false;  // sieve has been completed
//	CBigNum p = vPrimes[nPrimeSeq];
//	if (bnFixedFactor % p == 0)
//	{
//		// Nothing in the sieve is divisible by this prime
//		nPrimeSeq++;
//		return true;
//	}
//	// Find the modulo inverse of fixed factor
//	CBigNum bnFixedInverse;
//	if (!BN_mod_inverse(&bnFixedInverse, &bnFixedFactor, &p, pctx))
//		return error("CSieveOfEratosthenes::Weave(): BN_mod_inverse of fixed factor failed for prime #%u=%u", nPrimeSeq, vPrimes[nPrimeSeq]);
//	CBigNum bnTwo = 2;
//	CBigNum bnTwoInverse;
//	if (!BN_mod_inverse(&bnTwoInverse, &bnTwo, &p, pctx))
//		return error("CSieveOfEratosthenes::Weave(): BN_mod_inverse of 2 failed for prime #%u=%u", nPrimeSeq, vPrimes[nPrimeSeq]);
//
//	// Weave the sieve for the prime
//	unsigned int nChainLength = TargetGetLength(nBits);
//
//	uint64 bnFixedInverseU64 = bnFixedInverse.getuint();
//	uint64 pU64 = bnFixedInverse.getuint();
//	uint64 bnTwoInverseU64 = bnTwoInverse.getuint();
//
//	if( bnFixedInverse > CBigNum(0xFFFFFFFFULL) )
//		__debugbreak();
//	if( pU64 > CBigNum(0xFFFFFFFFULL) )
//		__debugbreak();
//	if( bnTwoInverse > CBigNum(0xFFFFFFFFULL) )
//		__debugbreak();
//
//	for (unsigned int nBiTwinSeq = 0; nBiTwinSeq < 2 * nChainLength; nBiTwinSeq++)
//	{
//		// Find the first number that's divisible by this prime
//		int nDelta = ((nBiTwinSeq % 2 == 0)? (-1) : 1);
//		//unsigned int nSolvedMultiplier = ((bnFixedInverse * (p - nDelta)) % p).getuint();
//		unsigned int nSolvedMultiplierX = ((bnFixedInverseU64 * (pU64 - nDelta)) % pU64);
//		unsigned int nSolvedMultiplier = (((bnFixedInverseU64%pU64) * ((pU64 - nDelta)%pU64)) % pU64);
//		if( nSolvedMultiplier != nSolvedMultiplierX )
//			__debugbreak();
//		if (nBiTwinSeq % 2 == 1)
//		{
//			uint64 backupX = bnFixedInverseU64;
//
//			bnFixedInverseU64 *= bnTwoInverseU64;//bnFixedInverse *= bnTwoInverse; // for next number in chain
//			if( bnFixedInverseU64 >= 0) //0x0000F00000000000ULL ) //(bnFixedInverseU64 / bnTwoInverseU64) != backupX )//bnFixedInverseU64 < backupX )
//			{
//				bnFixedInverseU64 = backupX;
//				// overflow occured, continue with big numbers
//				//__debugbreak();
//				// update the big integers (not all need updating, check later)
//				bnFixedInverse.setuint64(bnFixedInverseU64);
//				p.setuint64(pU64);
//				bnTwoInverse.setuint64(bnTwoInverseU64);
//				// finish current loop in bignum mode
//				bnFixedInverse *= bnTwoInverse;
//				unsigned int nPrime = vPrimes[nPrimeSeq];
//				if (nBiTwinSeq < nChainLength)
//					for (unsigned int nVariableMultiplier = nSolvedMultiplier; nVariableMultiplier < nSieveSize; nVariableMultiplier += nPrime)
//						vfCompositeBiTwin[nVariableMultiplier] = true;
//				if (((nBiTwinSeq & 1u) == 0))
//					for (unsigned int nVariableMultiplier = nSolvedMultiplier; nVariableMultiplier < nSieveSize; nVariableMultiplier += nPrime)
//						vfCompositeCunningham1[nVariableMultiplier] = true;
//				if (((nBiTwinSeq & 1u) == 1u))
//					for (unsigned int nVariableMultiplier = nSolvedMultiplier; nVariableMultiplier < nSieveSize; nVariableMultiplier += nPrime)
//						vfCompositeCunningham2[nVariableMultiplier] = true;
//				nBiTwinSeq++;
//				// continue loop in bignum mode
//				for (;nBiTwinSeq < 2 * nChainLength; nBiTwinSeq++)
//				{
//					int nDelta = ((nBiTwinSeq % 2 == 0)? (-1) : 1);
//					unsigned int nSolvedMultiplier = ((bnFixedInverse * (p - nDelta)) % p).getuint();
//					if (nBiTwinSeq % 2 == 1)
//						bnFixedInverse *= bnTwoInverse;
//					unsigned int nPrime = vPrimes[nPrimeSeq];
//					if (nBiTwinSeq < nChainLength)
//						for (unsigned int nVariableMultiplier = nSolvedMultiplier; nVariableMultiplier < nSieveSize; nVariableMultiplier += nPrime)
//							vfCompositeBiTwin[nVariableMultiplier] = true;
//					if (((nBiTwinSeq & 1u) == 0))
//						for (unsigned int nVariableMultiplier = nSolvedMultiplier; nVariableMultiplier < nSieveSize; nVariableMultiplier += nPrime)
//							vfCompositeCunningham1[nVariableMultiplier] = true;
//					if (((nBiTwinSeq & 1u) == 1u))
//						for (unsigned int nVariableMultiplier = nSolvedMultiplier; nVariableMultiplier < nSieveSize; nVariableMultiplier += nPrime)
//							vfCompositeCunningham2[nVariableMultiplier] = true;
//				}
//
//				nPrimeSeq++;
//				return true;
//
//				//__debugbreak();
//			}
//		}
//
//		unsigned int nPrime = vPrimes[nPrimeSeq];
//		if (nBiTwinSeq < nChainLength)
//			for (unsigned int nVariableMultiplier = nSolvedMultiplier; nVariableMultiplier < nSieveSize; nVariableMultiplier += nPrime)
//				vfCompositeBiTwin[nVariableMultiplier] = true;
//		if (((nBiTwinSeq & 1u) == 0))
//			for (unsigned int nVariableMultiplier = nSolvedMultiplier; nVariableMultiplier < nSieveSize; nVariableMultiplier += nPrime)
//				vfCompositeCunningham1[nVariableMultiplier] = true;
//		if (((nBiTwinSeq & 1u) == 1u))
//			for (unsigned int nVariableMultiplier = nSolvedMultiplier; nVariableMultiplier < nSieveSize; nVariableMultiplier += nPrime)
//				vfCompositeCunningham2[nVariableMultiplier] = true;
//	}
//	nPrimeSeq++;
//	return true;
//}

#define fastInitBignum(bignumVar, bignumData) \
	bignumVar.d = (unsigned int*)bignumData; \
	bignumVar.dmax = 0x200/4; \
	bignumVar.flags = BN_FLG_STATIC_DATA; \
	bignumVar.neg = 0; \
	bignumVar.top = 1; 

//
//// Weave sieve for the next prime in table
//// Return values:
////   True  - weaved another prime; nComposite - number of composites removed
////   False - sieve already completed
//bool CSieveOfEratosthenes::WeaveFast2()
//{
//	if (nPrimeSeq >= vPrimesSize || vPrimes[nPrimeSeq] >= nSieveSize)
//		return false;  // sieve has been completed
//	//CBigNum p = vPrimes[nPrimeSeq];
//	// init some required bignums on the stack (no dynamic allocation at all)
//	BIGNUM bn_p;
//	BIGNUM bn_tmp;
//	BIGNUM bn_fixedInverse;
//	BIGNUM bn_twoInverse;
//	//BIGNUM bn_delta;
//	uint32 bignumData_p[0x200/4];
//	uint32 bignumData_tmp[0x200/4];
//	uint32 bignumData_fixedInverse[0x200/4];
//	uint32 bignumData_twoInverse[0x200/4];
//	//uint32 bignumData_delta[0x200/4];
//	fastInitBignum(bn_p, bignumData_p);
//	fastInitBignum(bn_tmp, bignumData_tmp);
//	fastInitBignum(bn_fixedInverse, bignumData_fixedInverse);
//	fastInitBignum(bn_twoInverse, bignumData_twoInverse);
//	//fastInitBignum(bn_delta, bignumData_delta);
//	BN_set_word(&bn_p, vPrimes[nPrimeSeq]);
//	BN_mod(&bn_tmp, &bnFixedFactor, &bn_p, pctx);
//
//	
//	//if (bnFixedFactor % p == 0)
//	if( BN_is_zero(&bn_tmp) )
//	{
//		// Nothing in the sieve is divisible by this prime
//		nPrimeSeq++;
//		return true;
//	}
//
//	//CBigNum p = CBigNum(vPrimes[nPrimeSeq]);
//	// debug: Code is correct until here
//
//	// Find the modulo inverse of fixed factor
//	if (!BN_mod_inverse(&bn_fixedInverse, &bnFixedFactor, &bn_p, pctx))
//		return error("CSieveOfEratosthenes::Weave(): BN_mod_inverse of fixed factor failed for prime #%u=%u", nPrimeSeq, vPrimes[nPrimeSeq]);
//	//CBigNum bnTwo = 2;
//	if (!BN_mod_inverse(&bn_twoInverse, &bn_constTwo, &bn_p, pctx))
//		return error("CSieveOfEratosthenes::Weave(): BN_mod_inverse of 2 failed for prime #%u=%u", nPrimeSeq, vPrimes[nPrimeSeq]);
//
//	// Weave the sieve for the prime
//	unsigned int nChainLength = TargetGetLength(nBits);
//	for (unsigned int nBiTwinSeq = 0; nBiTwinSeq < 2 * nChainLength; nBiTwinSeq++)
//	{
//		// Find the first number that's divisible by this prime
//		//int nDelta = (((nBiTwinSeq&1) == 0)? (-1) : 1);
//		//unsigned int nSolvedMultiplier = ((bnFixedInverse * (p - nDelta)) % p).getuint();
//		//if (nBiTwinSeq % 2 == 1)
//		//	bnFixedInverse *= bnTwoInverse; // for next number in chain
//
//		/*if( nDelta > 0 )
//			BN_set_word(&bn_delta, nDelta);
//		else
//		{
//			BN_set_word(&bn_delta, -nDelta);
//			bn_delta.neg = 1;
//		}*/
//
//		//unsigned int nSolvedMultiplier = ((bnFixedInverse * (p - nDelta)) % p).getuint();
//		//if (nBiTwinSeq % 2 == 1)
//		//	bnFixedInverse *= bnTwoInverse; // for next number in chain
//
//		// maybe just use this method below: BN_sub_word() (since delta is actually a word)
//		//BN_sub(&bn_tmp, &bn_p, &bn_delta);
//		BN_copy(&bn_tmp, &bn_p);
//		if( (nBiTwinSeq&1) == 0 )
//			BN_add_word(&bn_tmp, 1);
//		else
//			BN_sub_word(&bn_tmp, 1);		
//		BN_mul(&bn_tmp, &bn_fixedInverse, &bn_tmp, pctx);
//		BN_mod(&bn_tmp, &bn_tmp, &bn_p, pctx);
//		unsigned int nSolvedMultiplier = BN_get_word(&bn_tmp);
//
//		if ((nBiTwinSeq & 1) == 1)
//			BN_mul(&bn_fixedInverse, &bn_fixedInverse, &bn_twoInverse, pctx);
//
//		//unsigned int nSolvedMultiplier = ((bnFixedInverse * (p - nDelta)) % p).getuint();
//		//if (nBiTwinSeq % 2 == 1)
//		//	bnFixedInverse *= bnTwoInverse; // for next number in chain
//
//
//		unsigned int nPrime = vPrimes[nPrimeSeq];
//		if (nBiTwinSeq < nChainLength)
//			for (unsigned int nVariableMultiplier = nSolvedMultiplier; nVariableMultiplier < nSieveSize; nVariableMultiplier += nPrime)
//				vfCompositeBiTwin[nVariableMultiplier] = true;
//		if (((nBiTwinSeq & 1u) == 0))
//			for (unsigned int nVariableMultiplier = nSolvedMultiplier; nVariableMultiplier < nSieveSize; nVariableMultiplier += nPrime)
//				vfCompositeCunningham1[nVariableMultiplier] = true;
//		if (((nBiTwinSeq & 1u) == 1u))
//			for (unsigned int nVariableMultiplier = nSolvedMultiplier; nVariableMultiplier < nSieveSize; nVariableMultiplier += nPrime)
//				vfCompositeCunningham2[nVariableMultiplier] = true;
//	}
//	nPrimeSeq++;
//	return true;
//}

// Weave sieve for the next prime in table
// Return values:
//   True  - weaved another prime; nComposite - number of composites removed
//   False - sieve already completed
bool CSieveOfEratosthenes::WeaveFastAll()
{
	//CBigNum p = vPrimes[nPrimeSeq];
	// init some required bignums on the stack (no dynamic allocation at all)
	BIGNUM bn_p;
	BIGNUM bn_tmp;
	BIGNUM bn_fixedInverse;
	BIGNUM bn_twoInverse;
	//BIGNUM bn_delta;
	uint32 bignumData_p[0x200/4];
	uint32 bignumData_tmp[0x200/4];
	uint32 bignumData_fixedInverse[0x200/4];
	uint32 bignumData_twoInverse[0x200/4];
	//uint32 bignumData_delta[0x200/4];
	fastInitBignum(bn_p, bignumData_p);
	fastInitBignum(bn_tmp, bignumData_tmp);
	fastInitBignum(bn_fixedInverse, bignumData_fixedInverse);
	fastInitBignum(bn_twoInverse, bignumData_twoInverse);
	//fastInitBignum(bn_delta, bignumData_delta);
	
	
	while( true )
	{
		if (nPrimeSeq >= vPrimesSize || vPrimes[nPrimeSeq] >= nSieveSize)
			return false;  // sieve has been completed

	
		BN_set_word(&bn_p, vPrimes[nPrimeSeq]);
		BN_mod(&bn_tmp, &bnFixedFactor, &bn_p, pctx);

		
		//if (bnFixedFactor % p == 0)
		if( BN_is_zero(&bn_tmp) )
		{
			// Nothing in the sieve is divisible by this prime
			nPrimeSeq++;
			continue;
		}

		//CBigNum p = CBigNum(vPrimes[nPrimeSeq]);
		// debug: Code is correct until here

		// Find the modulo inverse of fixed factor
		if (!BN_mod_inverse(&bn_fixedInverse, &bnFixedFactor, &bn_p, pctx))
			return error("CSieveOfEratosthenes::Weave(): BN_mod_inverse of fixed factor failed for prime #%u=%u", nPrimeSeq, vPrimes[nPrimeSeq]);
		//CBigNum bnTwo = 2;
		if (!BN_mod_inverse(&bn_twoInverse, &bn_constTwo, &bn_p, pctx))
			return error("CSieveOfEratosthenes::Weave(): BN_mod_inverse of 2 failed for prime #%u=%u", nPrimeSeq, vPrimes[nPrimeSeq]);

		
		uint64 pU64 = (uint64)vPrimes[nPrimeSeq];
		uint64 fixedInverseU64 = BN_get_word(&bn_fixedInverse);
		uint64 twoInverseU64 = BN_get_word(&bn_twoInverse);
		// Weave the sieve for the prime
		unsigned int nChainLength = TargetGetLength(nBits);
		for (unsigned int nBiTwinSeq = 0; nBiTwinSeq < 2 * nChainLength; nBiTwinSeq++)
		{
			// Find the first number that's divisible by this prime
			//BN_copy(&bn_tmp, &bn_p);
			//if( (nBiTwinSeq&1) == 0 )
			//	BN_add_word(&bn_tmp, 1);
			//else
			//	BN_sub_word(&bn_tmp, 1);		
			//BN_mul(&bn_tmp, &bn_fixedInverse, &bn_tmp, pctx);
			//BN_mod(&bn_tmp, &bn_tmp, &bn_p, pctx);
			//unsigned int nSolvedMultiplier = BN_get_word(&bn_tmp);

			uint64 nSolvedMultiplier;
			if( (nBiTwinSeq&1) == 0 )
				nSolvedMultiplier = ((fixedInverseU64) * (pU64 + 1ULL)) % pU64;
			else
				nSolvedMultiplier = ((fixedInverseU64) * (pU64 - 1ULL)) % pU64;

			//if( nSolvedMultiplier != nSolvedMultiplier2 )
			//	__debugbreak();

			if (nBiTwinSeq % 2 == 1)
			{
				fixedInverseU64 = (fixedInverseU64*twoInverseU64)%pU64;
			}
			unsigned int nPrime = vPrimes[nPrimeSeq];
			if (nBiTwinSeq < nChainLength)
				for (unsigned int nVariableMultiplier = nSolvedMultiplier; nVariableMultiplier < nSieveSize; nVariableMultiplier += nPrime)
					vfCompositeBiTwin[nVariableMultiplier>>3] |= 1<<(nVariableMultiplier&7);
			if (((nBiTwinSeq & 1u) == 0))
				for (unsigned int nVariableMultiplier = nSolvedMultiplier; nVariableMultiplier < nSieveSize; nVariableMultiplier += nPrime)
					vfCompositeCunningham1[nVariableMultiplier>>3] |= 1<<(nVariableMultiplier&7);
			if (((nBiTwinSeq & 1u) == 1u))
				for (unsigned int nVariableMultiplier = nSolvedMultiplier; nVariableMultiplier < nSieveSize; nVariableMultiplier += nPrime)
					vfCompositeCunningham2[nVariableMultiplier>>3] |= 1<<(nVariableMultiplier&7);
		}
		nPrimeSeq++;
	}
	return false; // never reached
}

//bool CSieveOfEratosthenes::WeaveAlt()
//{
//	if (nPrimeSeq >= vPrimesSize || vPrimes[nPrimeSeq] >= nSieveSize)
//		return false;  // sieve has been completed
//	jhBignum_t bn_p;
//	Bignum_SetUINT32(&bn_p, vPrimes[nPrimeSeq]);
//	
//	//CBigNum p = vPrimes[nPrimeSeq];
//
//	
//	//if (bnFixedFactor % p == 0)
//	if( Bignum_CanDivideWithoutRemainder(&_bnFixedFactor, &bn_p) )
//	{
//		// Nothing in the sieve is divisible by this prime
//		nPrimeSeq++;
//		return true;
//	}
//	// Find the modulo inverse of fixed factor
//	CAutoBN_CTX pctx;
//	CBigNum bnFixedInverse;
//	if (!BN_mod_inverse(&bnFixedInverse, &bnFixedFactor, &p, pctx))
//		return error("CSieveOfEratosthenes::Weave(): BN_mod_inverse of fixed factor failed for prime #%u=%u", nPrimeSeq, vPrimes[nPrimeSeq]);
//	CBigNum bnTwo = 2;
//	CBigNum bnTwoInverse;
//	if (!BN_mod_inverse(&bnTwoInverse, &bnTwo, &p, pctx))
//		return error("CSieveOfEratosthenes::Weave(): BN_mod_inverse of 2 failed for prime #%u=%u", nPrimeSeq, vPrimes[nPrimeSeq]);
//
//	// Weave the sieve for the prime
//	unsigned int nChainLength = TargetGetLength(nBits);
//	for (unsigned int nBiTwinSeq = 0; nBiTwinSeq < 2 * nChainLength; nBiTwinSeq++)
//	{
//		// Find the first number that's divisible by this prime
//		int nDelta = ((nBiTwinSeq % 2 == 0)? (-1) : 1);
//		unsigned int nSolvedMultiplier = ((bnFixedInverse * (p - nDelta)) % p).getuint();
//		if (nBiTwinSeq % 2 == 1)
//			bnFixedInverse *= bnTwoInverse; // for next number in chain
//
//		unsigned int nPrime = vPrimes[nPrimeSeq];
//		if (nBiTwinSeq < nChainLength)
//			for (unsigned int nVariableMultiplier = nSolvedMultiplier; nVariableMultiplier < nSieveSize; nVariableMultiplier += nPrime)
//				vfCompositeBiTwin[nVariableMultiplier] = true;
//		if (((nBiTwinSeq & 1u) == 0))
//			for (unsigned int nVariableMultiplier = nSolvedMultiplier; nVariableMultiplier < nSieveSize; nVariableMultiplier += nPrime)
//				vfCompositeCunningham1[nVariableMultiplier] = true;
//		if (((nBiTwinSeq & 1u) == 1u))
//			for (unsigned int nVariableMultiplier = nSolvedMultiplier; nVariableMultiplier < nSieveSize; nVariableMultiplier += nPrime)
//				vfCompositeCunningham2[nVariableMultiplier] = true;
//	}
//	nPrimeSeq++;
//	return true;
//}
