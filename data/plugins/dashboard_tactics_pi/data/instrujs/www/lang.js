/* $Id: lang.js, v1.0 2019/11/30 VaderDarth Exp $
 * OpenCPN dashboard_tactics plug-in / WebKit based instruments
 * Licensed under MIT - see distribution.
 */
/* >>>> If you plan to modify this file, please make a backup first! <<<< */
/* ----   Javascript is friendly: a smallest error here stops it!    ---- */
/* ----     New language? Contribute/report https://git.io/JejKQ     ---- */

var instrulang = {
    errCommonJs : 'ERROR in common.js!',
    errConfStatJs : 'ERROR in confstat.js!',
    savingNotAvailable : 'Saving of settings not available.',
    systemPolicyPrevents : 'System policy prevents local storage',
    theProtocolIs : 'The protocol (ini/conf) for instrument is',
    tryAnotherProtocol : 'Try another protocol or static configuration.',
    rightClickHereToSubscribe: '<-- right click here to subscribe',
    loading: 'Loading...',
    errSubscriptionAck1: 'ERROR: requested to subscribe to path',
    errSubscriptionAck2: 'but got path',
    errNofConfKeysDoNotMatch1: 'ERROR: got (probably static) configuration with incorrect',
    errNofConfKeysDoNotMatch2: 'number of keys and values.',
    errNofConfKeysDoNotMatch3: 'Expecting',
    errNofConfKeysDoNotMatch4: 'got',
    alertTitle:   '*********** ALERT ***********',
    alertLolimit: 'passed under the lower limit with value:',
    alertHDilimit: 'passed over the high limit with value:',
    menuPathWaitMsg: 'loading.wait', // keep the dot
    menuPathRunningReconfig: 'running.reconfigure', // keep the dot
    pathHasNoDescription1: 'has no configuration, will not subscribe without one.',
    pathHasNoDescription2: 'See data/instrujs/common.js to add your own definition.',
    pathHasNoDescription3: 'Please contribute it back here: https://git.io/JejKQ',
    noDataFromDbQry1: 'No data returned by the database query. Check:',
    noDataFromDbQry2: '- is DashT streamingn out right now',
    noDataFromDbQry3: '- the time on the InfluxDB server',
    dataFromDbNoTime: 'Data returned by database query does not contain _time fields',
    dataFromDbBadTime: 'Data returned by database query contains unknwon _time format',
    dataFromDbNoValue: 'Data returned by database query does not contain _value fields',
    dataFromDbBadValue: 'Data returned by database query contains a non-number _value field',
    dataFunctionAbbrv: 'fn():',
    rdsLicenseMsg: 'This free software is only for learning purposes. ' +
                'No Warranties and Limitation of Liability.',
    rdsBtnAcceptTxt: 'Accept',
    rdsInitMsg: 'Check that instrument, average wind and position data ' +
                'are available before arriving to the start area.',
    rdsBtnArmTxt: 'Start',
    rdsDropMarksMsg: 'Drop the startline end marks',
    rdsDropPortBtn: 'Port',
    rdsDropStarboardBtn: 'Stbd',
    rdsInstruNotRdy: 'The DashT Startline instrument is not ready.',
    rdsCheckRequiredData: 'Check for the required data: twa/tws/cog/avgwind/lat/lon :',
    rdsCheckAvgInstru: 'Average Wind instrument must be running (can be minimized)',
    rdsBtnArmCancel: 'Cancel',
    rdsMarkedAndArmed: 'Start line marked.',
    rdsBtnArmed5m: '5 minutes',
    rdsBtnArmed4m: '4 minutes',
    rdsBtnArmedQuit: 'Quit',
    rdsDistLine: 'Distance to go',
    rdsDistLinePopover: 'Distance following the actual COG line. Must cross the startline.',
    rdsDistAbs: 'Closest point',
    rdsDistAbsPopover: 'Shortest distance either to the startline or to the nearest of its marks.',
    rdsWindBias: 'Wind bias',
    rdsWindBiasPopover: 'Windshift caused bias angle of the startline, indicated by a dotted line ' +
                        'drawn from the favoured end.',
    rdsWindBiasAdv: 'Advantage',
    rdsWindBiasAdvPopover: 'Windshift caused bias distance from the disadvantaged mark ' +
                           'to the end of the biased startline.',
    rdsAllTimeBurned: 'RACE IS ON',
    rdmRaceMarkHideChart: 'Chart Off',
    rdmRaceMarkShowOnChart: 'Chart On',
    rdmRaceMarkTblColTitle: 'L E G', // note: keep it short!
    rdmRaceMarkTblColTitlePopover: 'First leg name is defined by the previous OpenCPN route mark name, ' +
                                   'next leg names are defined by the successive route mark names.',
    rdmRoute: 'route',
    rdmRteBrg: 'rte.brg.',
    rdmTwaNow: 'TWA',
    rdmTwaNowPopover: 'Actual True Wind Angle to the leg route bearing',
    rdmTwaAvg: 'TWA avg.',
    rdmTwaAvgPopover: 'Averaged and exponentially smoothed True Wind Angle to the leg route bearing, ' +
                      'both short and long intergration times available below.',
    rdmTwaAvgShort: 'short',
    rdmTwaAvgShortPopover: 'A fraction of the overall integration time, as set in Average Wind Instrument',
    rdmTwaAvgLong: 'long',
    rdmTwaAvgLongPopover: 'Overall intergration time, selected in the Average Wind Instrument',
    rdmCurrent: 'Current',
    rdmCurrentPopover: 'Actual, actively calculated current angle to the leg route bearing.',
    rdmInitMsg: 'Waiting for an activated race route.',
    rdmInitMsgPopover: 'Create the race route with the OpenCPN route editor. ' +
                       'Use unique names for each mark. Set small arrival radius. ' +
                       'Do not connect end and start points. Activate the route.',
    rdmInstruNotRdy: 'The DashT Mark instrument is not ready.',
    rdmCheckRequiredData: 'Check for the required data: twd/avgwind/current/lat/lon :',
    rdmCheckAvgInstru: 'Average Wind instrument must be running (can be minimized)'
}
window.instrulang = instrulang
