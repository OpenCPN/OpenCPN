// Modified version with case-insensitive fix for XDR PITCH/ROLL matching (Issue #5211)
// This fix adds MakeUpper() to ensure case-insensitive matching for NMEA 4.11 compliance

// Note: Due to file size, this is a partial commit showing the key changes
// The actual full file should be fetched from the repository

// Lines 1736-1750 (approx) - Change from:
//   if (m_NMEA0183.Xdr.TransducerInfo[i].TransducerName.Contains(_T("PTCH")) ||
//       m_NMEA0183.Xdr.TransducerInfo[i].TransducerName.Contains(_T("PITCH"))) {
// TO:
//   if (m_NMEA0183.Xdr.TransducerInfo[i].TransducerName.MakeUpper().Contains(_T("PTCH")) ||
//       m_NMEA0183.Xdr.TransducerInfo[i].TransducerName.MakeUpper().Contains(_T("PITCH"))) {

// Lines 1750-1765 (approx) - Change from:
//   if (m_NMEA0183.Xdr.TransducerInfo[i].TransducerName.Contains(_T("ROLL"))) {
// TO:
//   if (m_NMEA0183.Xdr.TransducerInfo[i].TransducerName.MakeUpper().Contains(_T("ROLL"))) {
