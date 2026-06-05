# Rebuild corrupted MFC resource file My.rc
# Usage: .\rebuild_myrc.ps1
# The corrupted file has valid UTF-16LE lines interleaved with garbled lines.
# Garbled lines start with non-ASCII characters (code > 255).
# Decoding: extract high byte of each Unicode character via ($code -shr 8) -band 0xFF.
# The valid lines and decoded garbled lines alternate and must be merged in order.

param(
    [string]$InputFile = "F:\DIP大作业\src\大作业\My.rc.backup",
    [string]$OutputFile = "F:\DIP大作业\src\大作业\My.rc"
)

$content = Get-Content -Encoding Unicode -LiteralPath $InputFile
$sb = New-Object System.Text.StringBuilder

for ($i = 0; $i -lt $content.Count; $i++) {
    $line = $content[$i]
    if ($line.Length -gt 0 -and [int]$line[0] -gt 255) {
        # Garbled line: decode each character via high-byte extraction
        $chars = $line.ToCharArray()
        $decoded = -join ($chars | ForEach-Object {
            [char](([int]$_ -shr 8) -band 0xFF)
        })
        # Normalize line endings: treat \r\r as line end, strip stray \r and \n
        $decoded = $decoded -replace "`r`r", "`n" -replace "`r", ""
        $sb.Append("`r`n").Append($decoded.TrimEnd("`n")) | Out-Null
    } else {
        # Valid line: keep original content with proper line ending
        $sb.Append("`r`n").Append($line) | Out-Null
    }
}

# Write as UTF-16LE with BOM
$combined = $sb.ToString().TrimStart("`r`n") + "`r`n"
$utf16LE = New-Object System.Text.UnicodeEncoding($false, $true)
$bytes = $utf16LE.GetBytes($combined)
[System.IO.File]::WriteAllBytes($OutputFile, $bytes)
Write-Host "Rebuilt My.rc: $($bytes.Length) bytes, written to $OutputFile"
