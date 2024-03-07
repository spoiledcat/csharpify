param([string]$version="8.0.101")

$workload="gaming_workload_$version.zip"

if (-not (Test-Path $workload -PathType Leaf)) {
	Write-Host "Download the $workload from the GitHub releases page of this repository."
	exit -1
}

Expand-Archive -Force -Path $workload -DestinationPath nupkg\
