# Batch Training Script for 7 Wonders Duel AI
# Run multiple AI vs AI games automatically with auto-optimization

Write-Host "==============================================`n" -ForegroundColor Cyan
Write-Host "   7 Wonders Duel - Batch Training Mode     `n" -ForegroundColor Cyan
Write-Host "==============================================`n" -ForegroundColor Cyan

# Get the correct path to the executable
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$solutionDir = Split-Path -Parent $scriptDir

# Try multiple possible locations for the executable
$possiblePaths = @(
    (Join-Path $solutionDir "Models\x64\Release\Core.exe"),
    (Join-Path $solutionDir "x64\Release\Core.exe"),
    (Join-Path $scriptDir "x64\Release\Core.exe"),
    (Join-Path $solutionDir "Core\x64\Release\Core.exe"),
    (Join-Path $solutionDir "out\build\x64-Release\Core\Core.exe")
)

$exePath = $null
foreach ($path in $possiblePaths) {
    if (Test-Path $path) {
        $exePath = $path
        break
    }
}

# Check if executable exists
if (-not $exePath) {
    Write-Host "ERROR: Core.exe not found in any of these locations:" -ForegroundColor Red
    foreach ($path in $possiblePaths) {
        Write-Host "  - $path" -ForegroundColor Yellow
    }
    Write-Host "`nPlease build the project in Release mode first!" -ForegroundColor Yellow
    Write-Host "Build > Configuration Manager > Release | x64`n" -ForegroundColor Yellow
    pause
    exit 1
}

Write-Host "Using executable: $exePath`n" -ForegroundColor Green

# Ask user what type of matchups to run
Write-Host "Choose training matchup type:" -ForegroundColor Yellow
Write-Host "[1] Britney vs Britney (Peaceful mirror match)"
Write-Host "[2] Spears vs Spears (Aggressive mirror match)"
Write-Host "[3] Britney vs Spears (Mixed strategies)"
Write-Host "[4] All matchups (comprehensive training)"
Write-Host ""
$matchupChoice = Read-Host "Choice (1-4)"

Write-Host ""
$gamesPerMatchup = Read-Host "How many games per matchup? (recommended: 10-50)"

if (-not $gamesPerMatchup) { $gamesPerMatchup = 10 }
$gamesPerMatchup = [int]$gamesPerMatchup

# Define matchups based on user choice
$matchups = @()
$playstylesToOptimize = @()

switch ($matchupChoice) {
    "1" { 
        $matchups = @(@{P1="Britney"; P2="Britney"; P1Num=1; P2Num=1; Count=$gamesPerMatchup})
        $playstylesToOptimize = @("Britney")
        Write-Host "`nRunning Britney vs Britney mirror matches...`n" -ForegroundColor Magenta
    }
    "2" { 
        $matchups = @(@{P1="Spears"; P2="Spears"; P1Num=2; P2Num=2; Count=$gamesPerMatchup})
        $playstylesToOptimize = @("Spears")
        Write-Host "`nRunning Spears vs Spears mirror matches...`n" -ForegroundColor Red
    }
    "3" { 
        $matchups = @(
            @{P1="Britney"; P2="Spears"; P1Num=1; P2Num=2; Count=$gamesPerMatchup},
            @{P1="Spears"; P2="Britney"; P1Num=2; P2Num=1; Count=$gamesPerMatchup}
        )
        $playstylesToOptimize = @("Britney", "Spears")
        Write-Host "`nRunning Britney vs Spears mixed matches...`n" -ForegroundColor Yellow
    }
    default { 
        $matchups = @(
            @{P1="Britney"; P2="Britney"; P1Num=1; P2Num=1; Count=$gamesPerMatchup},
            @{P1="Spears"; P2="Spears"; P1Num=2; P2Num=2; Count=$gamesPerMatchup},
            @{P1="Britney"; P2="Spears"; P1Num=1; P2Num=2; Count=$gamesPerMatchup},
            @{P1="Spears"; P2="Britney"; P1Num=2; P2Num=1; Count=$gamesPerMatchup}
        )
        $playstylesToOptimize = @("Britney", "Spears")
        Write-Host "`nRunning ALL matchup types...`n" -ForegroundColor Cyan
    }
}

$totalGames = ($matchups | Measure-Object -Property Count -Sum).Sum
$currentGame = 0
$startTime = Get-Date

Write-Host "Total games to run: $totalGames`n" -ForegroundColor Yellow

foreach ($matchup in $matchups) {
    $p1 = $matchup.P1
    $p2 = $matchup.P2
    $p1Num = $matchup.P1Num
    $p2Num = $matchup.P2Num
    $count = $matchup.Count
    
    Write-Host "`n=== Running $count games: $p1 vs $p2 ===" -ForegroundColor Green
    
    for ($i = 1; $i -le $count; $i++) {
        $currentGame++
        $percentComplete = [math]::Round(($currentGame / $totalGames) * 100, 1)
        $elapsed = (Get-Date) - $startTime
        $avgTimePerGame = if ($currentGame -gt 1) { $elapsed.TotalSeconds / ($currentGame - 1) } else { 0 }
        $remainingGames = $totalGames - $currentGame
        $estimatedRemaining = [TimeSpan]::FromSeconds($avgTimePerGame * $remainingGames)
        
        Write-Host "[$currentGame/$totalGames - $percentComplete%] $p1 vs $p2 (Game $i/$count) " -NoNewline -ForegroundColor White
        if ($avgTimePerGame -gt 0) {
            Write-Host "ETA: $($estimatedRemaining.ToString('mm\:ss'))" -ForegroundColor Gray
        } else {
            Write-Host ""
        }
        
        # Create input for automated AI vs AI game
        # Mode 3 = AI vs AI
        # Then P1 playstyle number
        # Then P2 playstyle number
        $input = "3`n$p1Num`n$p2Num`n"
        
        # Run game with PowerShell-compatible input redirection
        try {
            # Create temp file with input
            $inputFile = [System.IO.Path]::GetTempFileName()
            Set-Content -Path $inputFile -Value $input -NoNewline
            
            # Run game and pipe input (PowerShell way)
            $output = Get-Content $inputFile | & $exePath 2>&1
            
            # Clean up temp file
            Remove-Item $inputFile -Force -ErrorAction SilentlyContinue
            
            # Check if game completed successfully (look for training data save message)
            $outputText = ($output -join "`n")
            if ($LASTEXITCODE -eq 0 -or $outputText -match "Training Data" -or $outputText -match "game_.*\.csv") {
                Write-Host "  ? Completed" -ForegroundColor Green
            } else {
                Write-Host "  ? Error" -ForegroundColor Red
                # Uncomment to see output for debugging:
                # Write-Host ($output | Select-Object -Last 5 | Out-String) -ForegroundColor Gray
            }
        } catch {
            Write-Host "  ? Failed: $_" -ForegroundColor Red
        }
        
        Start-Sleep -Milliseconds 200
    }
}

$totalTime = (Get-Date) - $startTime

Write-Host "`n==============================================`n" -ForegroundColor Cyan
Write-Host "   Batch Training Complete!                 `n" -ForegroundColor Cyan
Write-Host "==============================================`n" -ForegroundColor Cyan

Write-Host "Total time: $($totalTime.ToString('hh\:mm\:ss'))" -ForegroundColor Yellow
Write-Host "Average time per game: $([math]::Round($totalTime.TotalSeconds / $totalGames, 1))s`n" -ForegroundColor Yellow

# Wait for files to be written
Start-Sleep -Milliseconds 500

# =============================================================================
# AUTOMATIC WEIGHT OPTIMIZATION
# =============================================================================

Write-Host "`n==============================================`n" -ForegroundColor Cyan
Write-Host "   Auto-Optimizing Weights...               `n" -ForegroundColor Cyan
Write-Host "==============================================`n" -ForegroundColor Cyan

$trainingDataDir = Join-Path $scriptDir "TrainingData"
$weightsDir = Join-Path $scriptDir "OptimizedWeights"

# Create weights directory if needed
if (-not (Test-Path $weightsDir)) {
    New-Item -ItemType Directory -Path $weightsDir | Out-Null
}

$summaryPaths = @{
    "Britney" = Join-Path $trainingDataDir "Britney\summary.csv"
    "Spears" = Join-Path $trainingDataDir "Spears\summary.csv"
    "Mixed" = Join-Path $trainingDataDir "Mixed\summary.csv"
}

# Function to get default weights for a style
function Get-DefaultWeights($style) {
    if ($style -eq "Britney") {
        return @{
            victoryPointValue = 1.2
            militaryPriority = 0.8
            sciencePriority = 1.2
            economyPriority = 1.3
            resourceValue = 1.1
            coinValue = 1.2
            wonderVPBonus = 1.3
            wonderMilitaryBonus = 1.0
            wonderEconomyBonus = 1.2
            opponentDenial = 0.9
            futureValueDiscount = 0.95
        }
    } else {
        return @{
            victoryPointValue = 0.9
            militaryPriority = 1.5
            sciencePriority = 0.9
            economyPriority = 1.0
            resourceValue = 1.0
            coinValue = 1.0
            wonderVPBonus = 1.1
            wonderMilitaryBonus = 1.4
            wonderEconomyBonus = 1.0
            opponentDenial = 1.3
            futureValueDiscount = 0.92
        }
    }
}

# Function to load existing optimized weights if they exist
function Get-OptimizedWeights($style) {
    $weightFile = Join-Path $weightsDir "${style}_optimized.weights"
    if (Test-Path $weightFile) {
        $weights = @{}
        Get-Content $weightFile | ForEach-Object {
            if ($_ -match '^(\w+)=([\d.]+)$') {
                $weights[$matches[1]] = [double]$matches[2]
            }
        }
        if ($weights.Count -gt 0) {
            return $weights
        }
    }
    return Get-DefaultWeights $style
}

$optimizationResults = @{}

foreach ($styleKey in $playstylesToOptimize) {
    $summaryPath = $summaryPaths[$styleKey]
    
    if (-not (Test-Path $summaryPath)) {
        Write-Host "? No data for $styleKey - skipping optimization" -ForegroundColor Yellow
        continue
    }
    
    Write-Host "`n=== Optimizing $styleKey ===" -ForegroundColor Green
    
    # Load training data - manually parse to avoid PowerShell CSV conflicts
    try {
        $gameRecords = @()
        $lines = Get-Content -Path $summaryPath -Encoding UTF8
        
        if ($lines.Count -lt 2) {
            Write-Host "  ? CSV file is empty or invalid" -ForegroundColor Yellow
            continue
        }
        
        # Skip header (first line)
        for ($i = 1; $i -lt $lines.Count; $i++) {
            $line = $lines[$i]
            if ([string]::IsNullOrWhiteSpace($line)) { continue }
            
            # Parse CSV line: Timestamp,P1_Playstyle,P2_Playstyle,Winner,VictoryType,TotalTurns,P1_FinalScore,P2_FinalScore
            $fields = $line -split ','
            if ($fields.Count -lt 8) { continue }
            
            $game = [PSCustomObject]@{
                Timestamp = $fields[0]
                P1_Playstyle = $fields[1]
                P2_Playstyle = $fields[2]
                Winner = $fields[3]
                VictoryType = $fields[4]
                TotalTurns = [int]$fields[5]
                P1_FinalScore = [int]$fields[6]
                P2_FinalScore = [int]$fields[7]
            }
            
            $gameRecords += $game
        }
        
        $totalGames = $gameRecords.Count
        
        Write-Host "  Successfully loaded $totalGames game records" -ForegroundColor Gray
    } catch {
        Write-Host "  ? Error loading CSV: $_" -ForegroundColor Yellow
        Write-Host "  Exception: $($_.Exception.Message)" -ForegroundColor Gray
        continue
    }
    
    if ($totalGames -lt 5) {
        Write-Host "  ? Only $totalGames games - need at least 5 for optimization" -ForegroundColor Yellow
        continue
    }
    
    # Calculate statistics
    $wins = 0
    $losses = 0
    $ties = 0
    $totalScore = 0
    $totalTurns = 0
    $militaryWins = 0
    $scientificWins = 0
    $civilianWins = 0
    
    foreach ($game in $gameRecords) {
        $won = $false
        if ($game.P1_Playstyle -eq $styleKey -and $game.Winner -eq "P1") { $won = $true }
        if ($game.P2_Playstyle -eq $styleKey -and $game.Winner -eq "P2") { $won = $true }
        
        if ($won) {
            $wins++
            if ($game.VictoryType -match "Military") { $militaryWins++ }
            elseif ($game.VictoryType -match "Scientific") { $scientificWins++ }
            else { $civilianWins++ }
        } elseif ($game.Winner -eq "Tie") {
            $ties++
        } else {
            $losses++
        }
        
        if ($game.P1_Playstyle -eq $styleKey) {
            $totalScore += [int]$game.P1_FinalScore
        } else {
            $totalScore += [int]$game.P2_FinalScore
        }
        $totalTurns += [int]$game.TotalTurns
    }
    
    $winRate = if ($totalGames -gt 0) { $wins / $totalGames } else { 0 }
    $avgScore = if ($totalGames -gt 0) { [math]::Round($totalScore / $totalGames, 1) } else { 0 }
    $avgTurns = if ($totalGames -gt 0) { [math]::Round($totalTurns / $totalGames, 1) } else { 0 }
    
    Write-Host "  Games: $totalGames | Win Rate: $([math]::Round($winRate * 100, 1))% | Avg Score: $avgScore" -ForegroundColor White
    Write-Host "  Victories: Military=$militaryWins, Scientific=$scientificWins, Civilian=$civilianWins" -ForegroundColor Gray
    
    # Load current weights (either optimized or default)
    $oldWeights = Get-OptimizedWeights $styleKey
    $weights = $oldWeights.Clone()
    
    # Determine learning rate based on performance
    $learningRate = if ($winRate -lt 0.4) { 0.2 } elseif ($winRate -gt 0.6) { 0.05 } else { 0.1 }
    
    Write-Host "  Learning rate: $learningRate (performance-based)" -ForegroundColor Gray
    
    # Apply victory-type based adjustments
    if ($wins -gt 0) {
        $militaryRatio = $militaryWins / $wins
        $scientificRatio = $scientificWins / $wins
        $civilianRatio = $civilianWins / $wins
        
        if ($militaryRatio -gt 0.5) {
            $weights.militaryPriority *= (1.0 + $learningRate * 0.5)
            $weights.wonderMilitaryBonus *= (1.0 + $learningRate * 0.3)
            Write-Host "  ? Increased military focus ($([math]::Round($militaryRatio * 100, 0))% of wins)" -ForegroundColor Red
        }
        
        if ($scientificRatio -gt 0.5) {
            $weights.sciencePriority *= (1.0 + $learningRate * 0.5)
            Write-Host "  ? Increased science focus ($([math]::Round($scientificRatio * 100, 0))% of wins)" -ForegroundColor Blue
        }
        
        if ($civilianRatio -gt 0.5) {
            $weights.victoryPointValue *= (1.0 + $learningRate * 0.5)
            $weights.wonderVPBonus *= (1.0 + $learningRate * 0.3)
            Write-Host "  ? Increased VP focus ($([math]::Round($civilianRatio * 100, 0))% of wins)" -ForegroundColor Green
        }
    }
    
    # Poor performance - rebalance toward neutral
    if ($winRate -lt 0.4) {
        Write-Host "  ? Rebalancing weights (low win rate)" -ForegroundColor Yellow
        $weights.militaryPriority = $weights.militaryPriority * 0.8 + 1.0 * 0.2
        $weights.sciencePriority = $weights.sciencePriority * 0.8 + 1.0 * 0.2
        $weights.economyPriority = $weights.economyPriority * 0.8 + 1.0 * 0.2
        $weights.victoryPointValue = $weights.victoryPointValue * 0.8 + 1.0 * 0.2
    }
    
    # Clamp weights to reasonable ranges [0.1, 3.0]
    foreach ($key in @($weights.Keys)) {
        if ($weights[$key] -lt 0.1) { $weights[$key] = 0.1 }
        if ($weights[$key] -gt 3.0) { $weights[$key] = 3.0 }
    }
    
    # Save optimized weights
    $weightFile = Join-Path $weightsDir "${styleKey}_optimized.weights"
    $content = @"
# Optimized weights for $styleKey
# Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
# Based on $totalGames games with $([math]::Round($winRate * 100, 1))% win rate

victoryPointValue=$($weights.victoryPointValue.ToString("F4"))
militaryPriority=$($weights.militaryPriority.ToString("F4"))
sciencePriority=$($weights.sciencePriority.ToString("F4"))
economyPriority=$($weights.economyPriority.ToString("F4"))
resourceValue=$($weights.resourceValue.ToString("F4"))
coinValue=$($weights.coinValue.ToString("F4"))
wonderVPBonus=$($weights.wonderVPBonus.ToString("F4"))
wonderMilitaryBonus=$($weights.wonderMilitaryBonus.ToString("F4"))
wonderEconomyBonus=$($weights.wonderEconomyBonus.ToString("F4"))
opponentDenial=$($weights.opponentDenial.ToString("F4"))
futureValueDiscount=$($weights.futureValueDiscount.ToString("F4"))
"@
    
    Set-Content -Path $weightFile -Value $content
    
    # Store results for summary
    $optimizationResults[$styleKey] = @{
        OldWeights = $oldWeights
        NewWeights = $weights
        WinRate = $winRate
        Games = $totalGames
        Changed = $false
    }
    
    # Check if weights actually changed
    foreach ($key in $weights.Keys) {
        $diff = [math]::Abs($weights[$key] - $oldWeights[$key])
        if ($diff -gt 0.001) {
            $optimizationResults[$styleKey].Changed = $true
            break
        }
    }
    
    Write-Host "  ? Saved to: $weightFile" -ForegroundColor Green
}

# =============================================================================
# OPTIMIZATION SUMMARY
# =============================================================================

Write-Host "`n==============================================`n" -ForegroundColor Cyan
Write-Host "   Optimization Summary                     `n" -ForegroundColor Cyan
Write-Host "==============================================`n" -ForegroundColor Cyan

foreach ($style in $optimizationResults.Keys) {
    $result = $optimizationResults[$style]
    
    Write-Host "`n=== $style ===" -ForegroundColor $(if ($style -eq "Britney") {"Magenta"} else {"Red"})
    Write-Host "Performance: $([math]::Round($result.WinRate * 100, 1))% win rate over $($result.Games) games" -ForegroundColor White
    
    if ($result.Changed) {
        Write-Host "`nKey Weight Changes:" -ForegroundColor Yellow
        
        $significantChanges = @()
        foreach ($key in $result.NewWeights.Keys) {
            $old = $result.OldWeights[$key]
            $new = $result.NewWeights[$key]
            $diff = $new - $old
            $percentChange = if ($old -ne 0) { ($diff / $old) * 100 } else { 0 }
            
            if ([math]::Abs($percentChange) -gt 1.0) {
                $arrow = if ($diff -gt 0) {"?"} else {"?"}
                $color = if ($diff -gt 0) {"Green"} else {"Red"}
                $significantChanges += "  $arrow ${key}: $($old.ToString('F3')) ? $($new.ToString('F3')) ($([math]::Round($percentChange, 1))%)"
            }
        }
        
        if ($significantChanges.Count -gt 0) {
            $significantChanges | ForEach-Object { Write-Host $_ -ForegroundColor Cyan }
        } else {
            Write-Host "  Minor adjustments only (< 1% changes)" -ForegroundColor Gray
        }
    } else {
        Write-Host "`n? Weights already optimal - no changes needed" -ForegroundColor Green
    }
}

Write-Host "`n==============================================`n" -ForegroundColor Cyan

Write-Host "Optimized weights saved to: $weightsDir`n" -ForegroundColor Yellow
Write-Host "Next steps:" -ForegroundColor White
Write-Host "  1. Weights will be automatically used in next training session" -ForegroundColor Gray
Write-Host "  2. Or manually update AIConfig.ixx for permanent changes" -ForegroundColor Gray
Write-Host "  3. Run more training to validate improvements`n" -ForegroundColor Gray

pause
