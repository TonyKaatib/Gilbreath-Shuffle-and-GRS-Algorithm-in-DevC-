#include <iostream>
#include <algorithm>
#include <random>

using namespace std;

//Here we will be defining a struct of a single Card, with rank and suit, and defining a card deck as a vector of this struct.
struct Card{
	int rank;
	int suit;
};

using Card_Deck = vector<Card>;

//Ditto, it verifies the colour of the suit...
bool isRed(const Card& card) {
    return card.suit == 1 || card.suit == 2; 
}

//Of which in this implementation we have: 1 - Hearts ♥ | 2 - Diamonds ♦ | 3 - Spades ♠ | 4 - Clubs ♣
string suitToString(int suit) {
    switch (suit) {
        case 1: return "H";
        case 2: return "D";
        case 3: return "S";
        case 4: return "C";
        default: return "?";
    }
}

string rankToString(int rank) {
    switch (rank) {
        case 1:  return "A";
        case 11: return "J";
        case 12: return "Q";
        case 13: return "K";
        default: return to_string(rank);
    }
}

//And we string all of this
string cardToString(const Card& card) {
    return rankToString(card.rank) + suitToString(card.suit);
}

//Ditto, we create a standard deck card of 52 cards with ranks and suits
Card_Deck createStandardDeck() {
    Card_Deck card_deck;
    card_deck.reserve(52);

    for (int suit = 1; suit <= 4; ++suit) {
        for (int rank = 1; rank <= 13; ++rank) {
            card_deck.push_back({rank, suit});
        }
    }
    return card_deck;
}

//For the purpose of a Gilbreath deck, we will need to create a card deck which alternates colours with each card on the deck
Card_Deck createAlternateColoursDeck(){
		Card_Deck red;
		Card_Deck black;
		Card_Deck card_deck;
		
		red.reserve(26);
		black.reserve(26);
		card_deck.reserve(52);

		for(int r = 1; r <= 13; r++){
			red.push_back({r, 1});
			black.push_back({r, 3});
			red.push_back({r, 2});
			black.push_back({r, 4});
		}

		for (size_t i = 0; i < red.size(); i++) {
	        card_deck.push_back(red[i]);
	        card_deck.push_back(black[i]);
		}
		
		return card_deck;
}

//Obviously we will want to print the deck that we want to see
void printDeck(const Card_Deck& card_deck, const string& title = "Deck"){
	cout<<title<<" ("<<card_deck.size()<<" cards):\n";
	for(int i = 0; i < card_deck.size(); i++){
		cout<<cardToString(card_deck[i]);
		if (i + 1 != card_deck.size()) {
            cout<<" ";
        }
	}
	cout<<endl;
}

//Because we will be using a Gilbert–Shannon–Reeds shuffle instead of the STL shuffle,
//we will define here a binomial split, since a GSR shuffle follows a binomial distribution
size_t binomialSplit(size_t n, mt19937& rng){
	binomial_distribution<size_t> dist(n, 0.5);
	return dist(rng);	
};

//Ditto, splitting a 52-card deck into two...
void splitDeck(Card_Deck& card_deck, Card_Deck& half_1, Card_Deck& half_2, size_t pos){
	half_1.assign(card_deck.begin(), card_deck.begin() + static_cast<ptrdiff_t>(pos));
	half_2.assign(card_deck.begin() + static_cast<ptrdiff_t>(pos), card_deck.end());
};

//...And mixing them together
Card_Deck interleaveHalves(Card_Deck& first_half, Card_Deck& second_half, mt19937& rng){
	Card_Deck final_deck;
	final_deck.reserve(first_half.size() + second_half.size());
	
	size_t i = 0;
    size_t j = 0;
    
    while(i < first_half.size() || j < second_half.size()){
    	size_t left_remaining = first_half.size() - i;	
    	size_t right_remaining = second_half.size() - j;	
    	size_t total_remaining = left_remaining + right_remaining;
    	
    	if(left_remaining == 0){
    		final_deck.push_back(second_half[j++]);
    		continue;
		}
		    	
    	if(right_remaining == 0){
    		final_deck.push_back(first_half[i++]);
    		continue;
		}
		
		uniform_int_distribution<size_t> dist(1, total_remaining);
	    size_t rn = dist(rng);
	    
	    if (rn <= left_remaining){
	    	final_deck.push_back(first_half[i++]);
		} else {
			final_deck.push_back(second_half[j++]);
		}
	}
    
	return final_deck;
};

//To make sure that we have a proper Gilbreath deck, inverting the deck will be needed.
Card_Deck GSRShuffle(Card_Deck& card_deck, mt19937& rng){
	size_t split = binomialSplit(card_deck.size(), rng);
    
	Card_Deck first_half;
	Card_Deck second_half;
	splitDeck(card_deck, first_half, second_half, split);
	return interleaveHalves(first_half, second_half, rng);
}

Card_Deck gilbreathShuffle(Card_Deck& card_deck, mt19937& rng){
	size_t split = binomialSplit(card_deck.size(), rng);
    
	Card_Deck first_half;
	Card_Deck second_half;
	splitDeck(card_deck, first_half, second_half, split);
	reverse(second_half.begin(), second_half.end());
	return interleaveHalves(first_half, second_half, rng);
}

//Ditto, also important to test Bayer's and Diaconi's demonstration that
//7 riffle shuffles are needed to randomize a deck
Card_Deck repeatGSRshuffle(Card_Deck& card_deck, int n, mt19937& rng){
	for (int i = 0; i <= n; i++) {
        card_deck = GSRShuffle(card_deck, rng);
    }
    return card_deck;
};

//Obviously, we need to know if our Gilbreath deck is in fact a Gilbreath deck
bool isGilbreathDeck(Card_Deck& card_deck){
	if (card_deck.size() % 2 != 0){
		return false;
	}
	
	for(size_t i = 0; i < card_deck.size(); i += 2){
		bool one_red = isRed(card_deck[i]);
		bool two_red = isRed(card_deck[i + 1]);
		
		if(one_red == two_red){
			return false;
		}
	}
	return true;
};

//And also to know it through reading it
void printGilbreathPairsReport(Card_Deck& card_deck){
	if(card_deck.size() % 2 != 0){
		cout<<"Deck size is odd, cannot test pairs.\n";
		return;
	}
	
	cout<<"Gilbreath pair report: \n";
	for(size_t i = 0; i < card_deck.size(); i += 2){
		bool ok = isRed(card_deck[i]) != isRed(card_deck[i + 1]);
		
		cout<<"["<<(i / 2) + 1<<"] "
		<<cardToString(card_deck[i])<<" "
		<<cardToString(card_deck[i + 1])<<" -> "
		<<(ok ? "OK" : "FAIL")<<"\n";
	}
	cout<<"\n";
}

//Ditto, here we start with our alternating colours deck, shuffle it with a Gilbreath shuffle,
//Then see if the deck follows the Gilbreath principle
bool runGilbreathExperiment(mt19937& rng, bool verbose = true){
	Card_Deck magic_deck = createAlternateColoursDeck();
	if(verbose){
		printDeck(magic_deck, "Initial alternating colours deck");
	}
	
	Card_Deck shuffled_magic_deck = gilbreathShuffle(magic_deck, rng);
	
	if(verbose){
		printDeck(shuffled_magic_deck, "After 1 GSR shuffle");
		printGilbreathPairsReport(shuffled_magic_deck);
	}
	
	bool passed = isGilbreathDeck(shuffled_magic_deck);
	
	if(verbose){
		cout<<"Gilbreath test: "
			<<(passed ? "PASSED" : "FAILED")
			<<"\n";
	}
	return passed;
};


//And we could also run it as many times as we want
void runGilbreathMonteCarlo(std::mt19937& rng, int trials) {
    int passed = 0;
    int failed = 0;

    for (int t = 0; t < trials; ++t) {
        if (runGilbreathExperiment(rng, false)) {
            ++passed;
        } else {
            ++failed;
        }
    }

    cout<<"Gilbreath Monte Carlo"<<endl;
    cout<<"Trials : "<<trials<<endl;
    cout<<"Passed : "<<passed<<endl;
    cout<<"Failed : "<<failed<<endl;
    cout<<"Rate : "<< (100.0 * static_cast<double>(passed) / static_cast<double>(trials))<< "%\n";
}

//And there you have it.
int main(int argv, char** argc){
	random_device rd;
    mt19937 rng(rd());
    
    runGilbreathExperiment(rng, true);
    runGilbreathMonteCarlo(rng, 100000);
    return 0;
}