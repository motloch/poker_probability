// Task: Calculate preflop win probabilities for head-to-head in Texas Holdem Poker.
//       Assume you know both hands and no folding.
// Design aim: Speed
#include <iostream>
#include<array>
#include<stdio.h>

const int LARGE = 256*256*16;

// How we encode cards
int cardToInt(char card){
  switch(card) {
    case '2':
      return 0;
    case '3':
      return 1;
    case '4':
      return 2;
    case '5':
      return 3;
    case '6':
      return 4;
    case '7':
      return 5;
    case '8':
      return 6;
    case '9':
      return 7;
    case 'T':
      return 8;
    case 'J':
      return 9;
    case 'Q':
      return 10;
    case 'K':
      return 11;
    case 'A':
      return 12;
  }
  std::cout << "Wrong value of card! Aborting";
  std::exit(1);
}

int suitToInt(char suit){
  switch(suit) {
    case 'H':
      return 0;
    case 'C':
      return 13;
    case 'S':
      return 26;
    case 'D':
      return 39;
  }
  std::cout << "Wrong value of card! Aborting";
  std::exit(1);
}

int encode(const char card[2]){
  return cardToInt(card[0]) + suitToInt(card[1]);
}

/* Given a pointer to the start of a boolean array of cards representing a single suit,
 * return the highest card of a straigh (or minus one, if there is none.)
 */
int getHighestCardOfStraightFlush(const bool* cardCounts)
{
  // start from the top, go towards the bottom
  int runningSum = 0;
  for(int i = 8; i <= 12; i++)
  {
    if(cardCounts[i])
      runningSum++;
  }

  if(runningSum == 5)
  {
    return 12;
  }
  
  for(int low = 7; low >= 0; low--)
  {
    runningSum += cardCounts[low]; 
    runningSum -= cardCounts[low + 5];

    if(runningSum == 5)
    {
      return low + 4;
    }
  }

  //A2345
  if((runningSum == 4) && cardCounts[12] && (not cardCounts[4]))
  {
    return 3;
  }

  return -1;
}

/* Given an array of number of different cards (2 -> A), return the highest card of a
 * straight (or minus one, if there is none).
 */
int getHighestCardOfStraight(const int cardCounts[13])
{
  // start from the top, go towards the bottom
  int runningSum = 0;
  for(int i = 8; i <= 12; i++)
  {
    if(cardCounts[i])
      runningSum++;
  }

  if(runningSum == 5)
  {
    return 12;
  }
  
  for(int low = 7; low >= 0; low--)
  {
    runningSum += bool(cardCounts[low]); 
    runningSum -= bool(cardCounts[low + 5]);

    if(runningSum == 5)
    {
      return low + 4;
    }
  }

  //A2345
  if((runningSum == 4) && (cardCounts[12]) && (cardCounts[4] == 0))
  {
    return 3;
  }

  return -1;
}

/* Given number of cards in individual suits, return which one has a flush (or -1 if none) */
int getFlushSuit(const int suitCounts[4])
{

  for(int j = 0; j <= 3; j++)
  {
    if(suitCounts[j] >= 5)
    {
      return j;
    }
  }

  return -1;
}

/* Return an integer representing strength of a hand. The higher, the stronger.
 *
 * General idea is for the returned value to be a sum of two parts
 * 1) multiple of LARGE, which quantifies "flush", "four of a kind", ...
 * 2) smaller number (less than LARGE) that orders within the ranking class
 *
 * Hand is inputted as an array of booleans (see above for encoding). Also passed two
 * integer arrays (number of cards of given kind, number of cards of given suit) to speed
 * up the calculations.
 */
int bestHand(const bool hand[52], const int cardCounts[13], const int suitCounts[4])
{
  int flushSuit = getFlushSuit(suitCounts);

  // straight flush (including Royal)
  if(flushSuit > -1)
  {
    int highestCardOfStraightFlush = getHighestCardOfStraightFlush(hand + 13*flushSuit);

    if(highestCardOfStraightFlush > -1){
      return 8*LARGE + highestCardOfStraightFlush;
    }
  }

  // Keep track of quads, triples, pairs and singles. If there are two pairs, twoA is the
  // higher one. If there are two triplets, we treat the lower as a pair for simplicity.
  // If there are three pairs, we treat the lowest as a single, for the same reason.
  // Singles are ordered in the array "one" from the largest to the lowest.
  int four = -1, three = -1, twoA = -1, twoB = -1;
  int one[7] = {-1};
  int nextOneIdx = 0;

  for(int i = 12; i >= 0; i--)
  {
    if(cardCounts[i] == 4)
    {
      four = i;
    }
    else if(cardCounts[i] == 3)
    {
      if(three == -1)
      {
        three = i;
      }
      else //Second triple -> we have full house. Treat the second triple as a pair.
      {
        twoA = std::max(twoA, i); 
      }
    }
    else if(cardCounts[i] == 2)
    {
      if(twoA == -1)
      {
        twoA = i;
      }
      else if(twoB == -1)
      {
        twoB = i;
      }
      else // we have two pairs already - treat the last pair as one of a kind
      {
        one[nextOneIdx] = i;
        nextOneIdx++;
      }
    }
    else if(cardCounts[i] == 1)
    {
        one[nextOneIdx] = i;
        nextOneIdx++;
    }
  }

  //four of a kind
  if(four > -1)
  {
      return 7*LARGE + 16*four + std::max(three, std::max(twoA, one[0]));
  }

  //full house
  if(three > -1 && twoA > -1)
  {
    return 6*LARGE + 16*three + twoA;
  }

  //flush
  if(flushSuit > -1)
  {
    int result = 5*LARGE;
    int scaler = 256*256;
    for(int i = 12; i >= 0; i--)
    {
      if(hand[13*flushSuit+i])
      {
        result += i*scaler;
        scaler /= 16; 

        if(scaler == 0)
        {
          return result;
        }
      }
    }
  }

  int highestCardOfStraight = getHighestCardOfStraight(cardCounts);

  //straight
  if(highestCardOfStraight > -1){
    return 4*LARGE + highestCardOfStraight;
  }

  //three of a kind
  if(three > -1)
  {
    return 3*LARGE + 256*three + 16*one[0] + one[1];
  }

  //two pair
  if(twoA > -1 and twoB > -1)
  {
    return 2*LARGE + 256*twoA + 16*twoB + one[0];
  }

  //pair
  if(twoA > -1)
  {
    return LARGE + 256*16*twoA + 256*one[0] + 16*one[1] + one[2];
  }

  //high card
  return 256*256*one[0] + 256*16*one[1] + 256*one[2] + 16*one[3] + one[4];

}

int whoWon(bool fullHand1[52], bool fullHand2[52], int cardCounts1[13], int cardCounts2[13], 
  int suitCounts1[4], int suitCounts2[4])
{
    // call subroutine to return integer describing how good the hand is
    int best1 = bestHand(fullHand1, cardCounts1, suitCounts1);
    int best2 = bestHand(fullHand2, cardCounts2, suitCounts2);

    // decide who won
    if(best1 > best2)
    {
        return 1;
    }
    else if (best2 > best1)
    {
        return 2;
    }
    return 0;
}

/* Check whether card i is in either of the two hands */
bool isIn(int i, int hand1[2], int hand2[2])
{
  return i == hand1[0] || i == hand1[1] || i == hand2[0] || i == hand2[1];
}

/* Calculate probabilities of two players winning head-to-head, based
 * on their pre-flop hands and assuming no folding.
 *
 * Returns p(win 1), p(win 2), p(draw)
 *
 * Each hand is encoded as an array of two integers (see above for encoding)
 */
std::array<float,3>  winProbabilitiesPreflop(int hand1[2], int hand2[2])
{
    int win1 = 0, win2 = 0, draw = 0;
    std::array<float,3> result;

    // encode cards in hand + common in a 1D array
    bool fullHand1[52] = {false}, fullHand2[52] = {false};

    // speeds up computation
    int cardCounts1[13] = {0}, cardCounts2[13] = {0};
    int suitCounts1[4] = {0}, suitCounts2[4] = {0};

    // cards in hand
    fullHand1[hand1[0]] = true;
    fullHand1[hand1[1]] = true;
    cardCounts1[hand1[0] % 13] += 1;
    cardCounts1[hand1[1] % 13] += 1;
    suitCounts1[hand1[0] / 13] += 1;
    suitCounts1[hand1[1] / 13] += 1;

    fullHand2[hand2[0]] = true;
    fullHand2[hand2[1]] = true;
    cardCounts2[hand2[0] % 13] += 1;
    cardCounts2[hand2[1] % 13] += 1;
    suitCounts2[hand2[0] / 13] += 1;
    suitCounts2[hand2[1] / 13] += 1;

    // iterate over all possible sets of five common cards
    for(int i1 = 0; i1 < 52; i1++)
    {
      if(isIn(i1, hand1, hand2))
        continue;

      fullHand1[i1] = true;
      fullHand2[i1] = true;
      cardCounts1[i1 % 13] += 1;
      cardCounts2[i1 % 13] += 1;
      suitCounts1[i1 / 13] += 1;
      suitCounts2[i1 / 13] += 1;

      for(int i2 = i1+1; i2 < 52; i2++)
      {
        if(isIn(i2, hand1, hand2))
          continue;
        fullHand1[i2] = true;
        fullHand2[i2] = true;
        cardCounts1[i2 % 13] += 1;
        cardCounts2[i2 % 13] += 1;
        suitCounts1[i2 / 13] += 1;
        suitCounts2[i2 / 13] += 1;

        for(int i3 = i2+1; i3 < 52; i3++)
        {
          if(isIn(i3, hand1, hand2))
            continue;
          fullHand1[i3] = true;
          fullHand2[i3] = true;
          cardCounts1[i3 % 13] += 1;
          cardCounts2[i3 % 13] += 1;
          suitCounts1[i3 / 13] += 1;
          suitCounts2[i3 / 13] += 1;

          for(int i4 = i3+1; i4 < 52; i4++)
          {
            if(isIn(i4, hand1, hand2))
              continue;
            fullHand1[i4] = true;
            fullHand2[i4] = true;
            cardCounts1[i4 % 13] += 1;
            cardCounts2[i4 % 13] += 1;
            suitCounts1[i4 / 13] += 1;
            suitCounts2[i4 / 13] += 1;

            for(int i5 = i4+1; i5 < 52; i5++)
            {
              if(isIn(i5, hand1, hand2))
                continue;
              fullHand1[i5] = true;
              fullHand2[i5] = true;
              cardCounts1[i5 % 13] += 1;
              cardCounts2[i5 % 13] += 1;
              suitCounts1[i5 / 13] += 1;
              suitCounts2[i5 / 13] += 1;

              switch(whoWon(fullHand1, fullHand2, cardCounts1, cardCounts2, suitCounts1, suitCounts2))
              {
                case(1):
                  win1++;
                  break;
                case(2):
                  win2++;
                  break;
                default:
                  draw++;
                  break;
              }

              fullHand1[i5] = false;
              fullHand2[i5] = false;
              cardCounts1[i5 % 13] -= 1;
              cardCounts2[i5 % 13] -= 1;
              suitCounts1[i5 / 13] -= 1;
              suitCounts2[i5 / 13] -= 1;
            }
            fullHand1[i4] = false;
            fullHand2[i4] = false;
            cardCounts1[i4 % 13] -= 1;
            cardCounts2[i4 % 13] -= 1;
            suitCounts1[i4 / 13] -= 1;
            suitCounts2[i4 / 13] -= 1;
          }
          fullHand1[i3] = false;
          fullHand2[i3] = false;
          cardCounts1[i3 % 13] -= 1;
          cardCounts2[i3 % 13] -= 1;
          suitCounts1[i3 / 13] -= 1;
          suitCounts2[i3 / 13] -= 1;
        }
        fullHand1[i2] = false;
        fullHand2[i2] = false;
        cardCounts1[i2 % 13] -= 1;
        cardCounts2[i2 % 13] -= 1;
        suitCounts1[i2 / 13] -= 1;
        suitCounts2[i2 / 13] -= 1;
      }
      fullHand1[i1] = false;
      fullHand2[i1] = false;
      cardCounts1[i1 % 13] -= 1;
      cardCounts2[i1 % 13] -= 1;
      suitCounts1[i1 / 13] -= 1;
      suitCounts2[i1 / 13] -= 1;
    }

    // collect results
    float s = win1 + win2 + draw;

    result[0] = win1/s;
    result[1] = win2/s;
    result[2] = draw/s;

    return result;
  
}

/* Calculates win probabilities for two particular hands. */
int main(){
  int hand1[2] = {encode("JD"), encode("TC")};
  int hand2[2] = {encode("7H"), encode("7S")};

  std::array<float,3>  result = winProbabilitiesPreflop(hand1, hand2);
  std::cout << result[0];
  std::cout << '\n';
  std::cout << result[1];
  std::cout << '\n';
  std::cout << result[2];
}
