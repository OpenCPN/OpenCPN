# Missing MBTiles with duplicate chart directories (#5566)

Use a disposable OpenCPN profile with OpenGL enabled. Do not use navigation data.
The helper requires only Python 3's standard library and creates artificial tiles.

1. Run `python3 test/missing_mbtiles.py create /tmp/missing-chart-fixtures`.
2. Add that directory **twice** in Charts and rebuild the chart database. If the
   UI rejects the second entry, close the test app and duplicate its `ChartDir`
   configuration entry with the next number, then reopen and rebuild.
3. Centre the chart at 51 N, 1 W and approximately 1:130,000. Confirm a blue tile
   displays, then close the app. Keep the resulting chart database unchanged.
4. Move both MBTiles files elsewhere. Restart, pan/zoom for at least 30 seconds,
   then close the app. Preserve its log before the next launch.
5. Run `python3 test/missing_mbtiles.py check /tmp/missing-chart-fixtures --log /path/to/opencpn.log`.

Before the fix, duplicate chart entries allow repeated attempts and SQLite
warning notifications accumulate. After the fix, each missing file is attempted
once per session and the checker exits successfully. A run with zero attempts
fails the checker too: it has not exercised the affected code.

Controls: restore only one file and verify that its tile displays while the other
file is attempted once. Restore both and restart: both should load without
missing-file errors. Repeat with the directory listed once. The fix must also
work with a pre-existing duplicate database, without requiring a rebuild.

The reported Windows log lists the same missing-chart directory at indices 11
and 32. This duplicate-directory reproduction also works on Linux; it does not
constitute a test of the reporter's Windows installation or graphics driver.
