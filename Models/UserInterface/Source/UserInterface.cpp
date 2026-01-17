	// React to tree-node changes/empties by refreshing age tree
	connect(m_gameListener.get(), &GameListenerBridge::treeNodeEmptiedSignal, this, [this](int ageIndex, int nodeIndex){
		Q_UNUSED(nodeIndex);
		Q_UNUSED(ageIndex);
		// DISABLED: The tree is already refreshed by AgeTreeWidget::handleLeafClicked()
		// Responding here causes duplicate/constant refreshing
		#if 0
		if (m_ageTreeWidget) m_ageTreeWidget->showAgeTree(ageIndex ==0 ?1 : ageIndex);
		#endif
	});
	connect(m_gameListener.get(), &GameListenerBridge::treeNodeChangedSignal, this,
		[this](int ageIndex, int nodeIndex, bool isAvailable, bool isVisible, bool isEmpty){
			Q_UNUSED(nodeIndex);
			Q_UNUSED(ageIndex);
			Q_UNUSED(isAvailable);
			Q_UNUSED(isVisible);
			Q_UNUSED(isEmpty);
			// DISABLED: The tree is already refreshed by AgeTreeWidget::handleLeafClicked()
			// Responding here causes duplicate/constant refreshing
			#if 0
			if (m_ageTreeWidget) m_ageTreeWidget->showAgeTree(ageIndex ==0 ?1 : ageIndex);
			#endif
		});
