export module Core.Game;

import Core.Preparation;
import Core.Board;
import Core.Node;

export namespace Core {
    // Expose the test preparation helper so main can call it to verify setup
    void preparation();
}
