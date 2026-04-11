//As this sits this code is finished and is awaiting to be used on the gui

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <sstream>

using namespace std;


class Card{

    private:

        string name;
        double balance;
        double apr;
        double minimumPayment;

    public:

        Card(){
            name = "";
            balance = 0;
            apr = 0;
            minimumPayment = 0;
        }

    void input(){

        cout << "Enter card name: ";
        getline(cin, name);

        cout << "Enter current balance: ";
        cin >> balance;

        cout << "Enter APR (%): ";
        cin >> apr;

        cout << "Enter minimum payment: ";
        cin >> minimumPayment;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    string getName() const{
       return name; 
    }
    double getBalance() const{
       return balance; 
    }
    double getAPR() const{
       return apr;
    }
    double getMinimumPayment() const{
       return minimumPayment; 
    }

    void addInterest(double amount){
        balance += amount;
    }

    double makePayment(double payment){

        if (payment > balance){
            double used = balance;
            balance = 0;
            return used;
        }
        balance -= payment;
        return payment;
    }

    bool isPaidOff() const{
        return balance <= 0.0001;
    }
};

enum Strategy{

    NO_ACTION,
    SNOWBALL,
    AVALANCHE
};

struct PayoffResult{

    string methodName;
    double totalPaid = 0;
    double totalInterest = 0;
    int months = 0;
};

bool allPaidOff(const vector<Card>& cards){

    for(const auto& card : cards){
        if(!card.isPaidOff()){
            return false;
        }
    }
    return true;
}

int getSnowballTarget(const vector<Card>& cards){

    int target = -1;
    for(int i = 0; i < cards.size(); i++){
        if(!cards[i].isPaidOff()){
            if(target == -1 || cards[i].getBalance() < cards[target].getBalance()){
                target = i;
            }
        }
    }
    return target;
}

int getAvalancheTarget(const vector<Card>& cards){

    int target = -1;
    for(int i = 0; i < cards.size(); i++){
        if(!cards[i].isPaidOff()){
            if(target == -1 || cards[i].getAPR() > cards[target].getAPR()){
                target = i;
            }
        }
    }
    return target;
}

  double getTotalMinimums(const vector<Card>& cards){

    double total = 0.0;
    for(const auto& card : cards){
      if(!card.isPaidOff()){
        total += card.getMinimumPayment();
      }
    }
    return total;
  }

  string money(double value){

    ostringstream out;
    out << "$" << fixed << setprecision(2) << value;
    return out.str();
  }

  void printSummary(const PayoffResult& noAction, const PayoffResult& snowball, const PayoffResult& avalanche){

    // Find least interest
    string bestMethod;
    double minInterest = snowball.totalInterest;
    bestMethod = "Snowball";

    if(avalanche.totalInterest < minInterest){
        minInterest = avalanche.totalInterest;
        bestMethod = "Avalanche";
    }

    // Find fastest payoff
    string fastestMethod;
    int minMonths = snowball.months;
    fastestMethod = "Snowball";

    if(avalanche.months < minMonths){
        minMonths = avalanche.months;
        fastestMethod = "Avalanche";
    }

    // Savings vs no action
    double snowballSavings = noAction.totalInterest - snowball.totalInterest;
    double avalancheSavings = noAction.totalInterest - avalanche.totalInterest;

    cout << "\n--------------------------------------------------\n";
    cout << "Best (Least Interest): " << bestMethod << endl;
    cout << "Fastest Payoff: " << fastestMethod << endl;
    cout << "Snowball Interest Saved vs No Action: " << money(snowballSavings) << endl;
    cout << "Avalanche Interest Saved vs No Action: " << money(avalancheSavings) << endl;
    cout << "--------------------------------------------------\n";
}

PayoffResult simulatePayoff(vector<Card> cards, Strategy strategy){

    PayoffResult result;

    if(strategy == NO_ACTION){
        result.methodName = "No Action";
    }else if (strategy == SNOWBALL){
        result.methodName = "Snowball";
    }else{
        result.methodName = "Avalanche";
    }

    const int MAX_MONTHS = 1200;
    double baseMonthlyPayment = getTotalMinimums(cards);

    while(!allPaidOff(cards) && result.months < MAX_MONTHS){

        result.months++;

        // Add monthly interest
        for(int i = 0; i < cards.size(); i++){
            if(!cards[i].isPaidOff()){
                double interest = cards[i].getBalance() * (cards[i].getAPR() / 100.0 / 12.0);
                cards[i].addInterest(interest);
                result.totalInterest += interest;
            }
        }

        if(strategy == NO_ACTION) {
            // Pay only each card's own minimum
            for(int i = 0; i < cards.size(); i++){
                if(!cards[i].isPaidOff()){
                    double payment = min(cards[i].getMinimumPayment(), cards[i].getBalance());
                    double used = cards[i].makePayment(payment);
                    result.totalPaid += used;
                }
            }
        }else{
            // Snowball / Avalanche:
            // Keep paying the original combined minimum total every month
            double moneyLeft = baseMonthlyPayment;

            // First pay all minimums
            for(int i = 0; i < cards.size(); i++){
                if(!cards[i].isPaidOff()) {
                    double payment = min(cards[i].getMinimumPayment(), cards[i].getBalance());
                    payment = min(payment, moneyLeft);

                    double used = cards[i].makePayment(payment);
                    result.totalPaid += used;
                    moneyLeft -= used;
                }
            }

            // Then roll extra freed-up money into target card
            while(moneyLeft > 0.0001 && !allPaidOff(cards)){
                int target = -1;

                if(strategy == SNOWBALL){
                    target = getSnowballTarget(cards);
                }else if (strategy == AVALANCHE){
                    target = getAvalancheTarget(cards);
                }

                if(target == -1){
                    break;
                }

                double used = cards[target].makePayment(moneyLeft);
                result.totalPaid += used;
                moneyLeft -= used;
            }
        }
    }
    return result;
}

void printComparisonTable(const PayoffResult& noAction, const PayoffResult& snowball, const PayoffResult& avalanche){

  cout << "\n" << left << setw(22) << "Category" << setw(20) << noAction.methodName << setw(20) << snowball.methodName << setw(20) << avalanche.methodName << endl;
  cout << string(82, '-') << endl;
  cout << left << setw(22) << "Months to Payoff" << setw(20) << noAction.months << setw(20) << snowball.months << setw(20) << avalanche.months << endl;
  cout << left << setw(22) << "Total Paid" << setw(20) << fixed << setprecision(2) << noAction.totalPaid << setw(20) << fixed << setprecision(2) << snowball.totalPaid << setw(20) << fixed << setprecision(2) << avalanche.totalPaid << endl;
  cout << left << setw(22) << "Total Interest" << setw(20) << fixed << setprecision(2) << noAction.totalInterest << setw(20) << fixed << setprecision(2) << snowball.totalInterest << setw(20) << fixed << setprecision(2) << avalanche.totalInterest << endl;
}



int main(){

    int numCards;
    vector<Card> cards;

    cout << "How many cards do you want to enter? ";
    cin >> numCards;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    for(int i = 0; i < numCards; i++) {
        cout << "\n--- Enter Card " << (i + 1) << " ---\n";
        Card temp;
        temp.input();
        cards.push_back(temp);
    }

    PayoffResult noActionResult = simulatePayoff(cards, NO_ACTION);
    PayoffResult snowballResult = simulatePayoff(cards, SNOWBALL);
    PayoffResult avalancheResult = simulatePayoff(cards, AVALANCHE);

    printComparisonTable(noActionResult, snowballResult, avalancheResult);
    printSummary(noActionResult, snowballResult, avalancheResult);

    return 0;
}
