#include <iostream>

import Core.Game;
import Core.Board; 


int main()
{
    std::cout << "Starting preparation test...\n";
    Core::preparation();
    std::cout << "Preparation finished.\n";
    Core::Board::getInstance().displayEntireBoard();
    return 0;
}
