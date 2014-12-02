#include "ni6602.h"

namespace pci6602
{

u32 PfiID[] = {
	ND_PFI_0, 
	ND_PFI_1, 
	ND_PFI_2, 
	ND_PFI_3, 
	ND_PFI_4, 
	ND_PFI_5, 
	ND_PFI_6, 
	ND_PFI_7, 
	ND_PFI_8, 
	ND_PFI_9, 
	ND_PFI_10,
	ND_PFI_11,
	ND_PFI_12,
	ND_PFI_13,
	ND_PFI_14,
	ND_PFI_15,
	ND_PFI_16,
	ND_PFI_17,
	ND_PFI_18,
	ND_PFI_19,
	ND_PFI_20,
	ND_PFI_21,
	ND_PFI_22,
	ND_PFI_23,
	ND_PFI_24,
	ND_PFI_25,
	ND_PFI_26,
	ND_PFI_27,
	ND_PFI_28,
	ND_PFI_29,
	ND_PFI_30,
	ND_PFI_31,
	ND_PFI_32,
	ND_PFI_33,
	ND_PFI_34,
	ND_PFI_35,
	ND_PFI_36,
	ND_PFI_37,
	ND_PFI_38,
	ND_PFI_39
};

u32 CounterID[8] = {
	ND_COUNTER_0,
	ND_COUNTER_1,
	ND_COUNTER_2,
	ND_COUNTER_3,
	ND_COUNTER_4,
	ND_COUNTER_5,
	ND_COUNTER_6,
	ND_COUNTER_7
};

u32 GateID[8] = {
	PfiID[38],
	PfiID[34],
	PfiID[30],
	PfiID[26],
	PfiID[22],
	PfiID[18],
	PfiID[14],
	PfiID[10]
};

u32 SourceID[8] = {
	PfiID[39],
	PfiID[35],
	PfiID[31],
	PfiID[27],
	PfiID[23],
	PfiID[19],
	PfiID[15],
	PfiID[11]
};

u32 AuxID[8] = {
	PfiID[37],
	PfiID[33],
	PfiID[29],
	PfiID[25],
	PfiID[21],
	PfiID[17],
	PfiID[13],
	PfiID[9]
};

u32 OutID[8] = {
	PfiID[36],
	PfiID[32],
	PfiID[28],
	PfiID[24],
	PfiID[20],
	PfiID[16],
	PfiID[12],
	PfiID[8]
};

u32 CounterOutID[] = {
	ND_GPCTR0_OUTPUT,
	ND_GPCTR1_OUTPUT,
	ND_GPCTR2_OUTPUT,
	ND_GPCTR3_OUTPUT,
	ND_GPCTR4_OUTPUT,
	ND_GPCTR5_OUTPUT,
	ND_GPCTR6_OUTPUT,
	ND_GPCTR7_OUTPUT,
};

u32 FilterID[] = {
	ND_5_MICROSECONDS, // Filters the signal using a 100 kHz filter. 
	ND_1_MICROSECOND, // Filters the signal using a 500 kHz filter. 
	ND_500_NANOSECONDS, // Filters the signal using a 1 MHz filter. 
	ND_100_NANOSECONDS, // Filters the signal using a 5 MHz filter. 
	ND_CONFIGURABLE_FILTER, // Filters the signal using a custom filter clock. See Select_Signal. 
	ND_NONE
};


const char* NIDAQ_Error[] = {
"unknownError",
"syntaxError", // 		-10001
"semanticsError", // 		-10002
"invalidValueError", // 		-10003
"valueConflictError", // 		-10004
"badDeviceError", // 		-10005
"badLineError", // 		-10006
"badChanError", // 		-10007
"badGroupError", // 		-10008
"badCounterError", // 		-10009
"badCountError", // 		-10010
"badIntervalError", // 		-10011
"badRangeError", // 		-10012
"badErrorCodeError", // 		-10013
"groupTooLargeError", // 		-10014
"badTimeLimitError", // 		-10015
"badReadCountError", // 		-10016
"badReadModeError", // 		-10017
"badReadOffsetError", // 		-10018
"badClkFrequencyError", // 		-10019
"badTimebaseError", // 		-10020
"badLimitsError", // 		-10021
"badWriteCountError", // 		-10022
"badWriteModeError", // 		-10023
"badWriteOffsetError", // 		-10024
"limitsOutOfRangeError", // 		-10025
"badBufferSpecificationError", // 		-10026
"badDAQEventError", // 		-10027
"badFilterCutoffError", // 		-10028
"obsoleteFunctionError", // 		-10029
"badBaudRateError", // 		-10030
"badChassisIDError", // 		-10031
"badModuleSlotError", // 		-10032
"invalidWinHandleError", // 		-10033
"noSuchMessageError", // 		-10034
"irrelevantAttributeError", // 		-10035
"badYearError", // 		-10036
"badMonthError", // 		-10037
"badDayError", // 		-10038
"stringTooLongError", // 		-10039
"badGroupSizeError", // 		-10040
"badTaskIDError", // 		-10041
"inappropriateControlCodeError", // 		-10042
"badDivisorError", // 		-10043
"badPolarityError", // 		-10044
"badInputModeError", // 		-10045
"badExcitationError", // 		-10046
"badConnectionTypeError", // 		-10047
"badExcitationTypeError", // 		-10048
"", // 		-10049
"badChanListError", // 		-10050
"", // 		-10051
"", // 		-10052
"", // 		-10053
"", // 		-10054
"", // 		-10055
"", // 		-10056
"", // 		-10057
"", // 		-10058
"", // 		-10059
"", // 		-10060
"", // 		-10061
"", // 		-10062
"", // 		-10063
"", // 		-10064
"", // 		-10065
"", // 		-10066
"", // 		-10067
"", // 		-10068
"", // 		-10069
"", // 		-10070
"", // 		-10071
"", // 		-10072
"", // 		-10073
"", // 		-10074
"", // 		-10075
"", // 		-10076
"", // 		-10077
"", // 		-10078
"badTrigSkipCountError", // 		-10079
"badGainError", // 		-10080
"badPretrigCountError", // 		-10081
"badPosttrigCountError", // 		-10082
"badTrigModeError", // 		-10083
"badTrigCountError", // 		-10084
"badTrigRangeError", // 		-10085
"badExtRefError", // 		-10086
"badTrigTypeError", // 		-10087
"badTrigLevelError", // 		-10088
"badTotalCountError", // 		-10089
"badRPGError", // 		-10090
"badIterationsError", // 		-10091
"lowScanIntervalError", // 		-10092
"fifoModeError", // 		-10093
"badCalDACconstError", // 		-10094
"badCalStimulusError", // 		-10095
"badCalibrationConstantError", // 		-10096
"badCalOpError", // 		-10097
"badCalConstAreaError", // 		-10098
"", // 		-10099
"badPortWidthError", // 		-10100
"", // 		-10101
"", // 		-10102
"", // 		-10103
"", // 		-10104
"", // 		-10105
"", // 		-10106
"", // 		-10107
"", // 		-10108
"", // 		-10109
"", // 		-10110
"", // 		-10111
"", // 		-10112
"", // 		-10113
"", // 		-10114
"", // 		-10115
"", // 		-10116
"", // 		-10117
"", // 		-10118
"", // 		-10119
"gpctrBadApplicationError", // 		-10120
"gpctrBadCtrNumberError", // 		-10121
"gpctrBadParamValueError", // 		-10122
"gpctrBadParamIDError", // 		-10123
"gpctrBadEntityIDError", // 		-10124
"gpctrBadActionError", // 		-10125
"gpctrSourceSelectError", // 		-10126
"badCountDirError", // 		-10127
"badGateOptionError", // 		-10128
"badGateModeError", // 		-10129
"badGateSourceError", // 		-10130
"badGateSignalError", // 		-10131
"badSourceEdgeError", // 		-10132
"badOutputTypeError", // 		-10133
"badOutputPolarityError", // 		-10134
"badPulseModeError", // 		-10135
"badDutyCycleError", // 		-10136
"badPulsePeriodError", // 		-10137
"badPulseDelayError", // 		-10138
"badPulseWidthError", // 		-10139
"badFOUTportError", // 		-10140
"badAutoIncrementModeError", // 		-10141
"", // 		-10142
"", // 		-10143
"", // 		-10144
"", // 		-10145
"", // 		-10146
"", // 		-10147
"", // 		-10148
"", // 		-10149
"CfgInvalidatedSysCalError", // 		-10150
"sysCalOutofDateError", // 		-10151
"", // 		-10152
"", // 		-10153
"", // 		-10154
"", // 		-10155
"", // 		-10156
"", // 		-10157
"", // 		-10158
"", // 		-10159
"", // 		-10160
"", // 		-10161
"", // 		-10162
"", // 		-10163
"", // 		-10164
"", // 		-10165
"", // 		-10166
"", // 		-10167
"", // 		-10168
"", // 		-10169
"", // 		-10170
"", // 		-10171
"", // 		-10172
"", // 		-10173
"", // 		-10174
"", // 		-10175
"", // 		-10176
"", // 		-10177
"", // 		-10178
"", // 		-10179
"badNotchFilterError", // 		-10180
"badMeasModeError", // 		-10181
"", // 		-10182
"", // 		-10183
"", // 		-10184
"", // 		-10185
"", // 		-10186
"", // 		-10187
"", // 		-10188
"", // 		-10189
"", // 		-10190
"", // 		-10191
"", // 		-10192
"", // 		-10193
"", // 		-10194
"", // 		-10195
"", // 		-10196
"", // 		-10197
"", // 		-10198
"", // 		-10199
"EEPROMreadError", // 		-10200
"EEPROMwriteError", // 		-10201
"EEPROMwriteProtectionError", // 		-10202
"EEPROMinvalidLocationError", // 		-10203
"EEPROMinvalidPasswordError", // 		-10204
"", // 		-10205
"", // 		-10206
"", // 		-10207
"", // 		-10208
"", // 		-10209
"", // 		-10210
"", // 		-10211
"", // 		-10212
"", // 		-10213
"", // 		-10214
"", // 		-10215
"", // 		-10216
"", // 		-10217
"", // 		-10218
"", // 		-10219
"", // 		-10220
"", // 		-10221
"", // 		-10222
"", // 		-10223
"", // 		-10224
"", // 		-10225
"", // 		-10226
"", // 		-10227
"", // 		-10228
"", // 		-10229
"", // 		-10230
"", // 		-10231
"", // 		-10232
"", // 		-10233
"", // 		-10234
"", // 		-10235
"", // 		-10236
"", // 		-10237
"", // 		-10238
"", // 		-10239
"noDriverError", // 		-10240
"oldDriverError", // 		-10241
"functionNotFoundError", // 		-10242
"configFileError", // 		-10243
"deviceInitError", // 		-10244
"osInitError", // 		-10245
"communicationsError", // 		-10246
"cmosConfigError", // 		-10247
"dupAddressError", // 		-10248
"intConfigError", // 		-10249
"dupIntError", // 		-10250
"dmaConfigError", // 		-10251
"dupDMAError", // 		-10252
"jumperlessBoardError", // 		-10253
"DAQCardConfError", // 		-10254
"remoteChassisDriverInitError", // 		-10255
"comPortOpenError", // 		-10256
"baseAddressError", // 		-10257
"dmaChannel1Error", // 		-10258
"dmaChannel2Error", // 		-10259
"dmaChannel3Error", // 		-10260
"userModeToKernelModeCallError", // 		-10261
"", // 		-10262
"", // 		-10263
"", // 		-10264
"", // 		-10265
"", // 		-10266
"", // 		-10267
"", // 		-10268
"", // 		-10269
"", // 		-10270
"", // 		-10271
"", // 		-10272
"", // 		-10273
"", // 		-10274
"", // 		-10275
"", // 		-10276
"", // 		-10277
"", // 		-10278
"", // 		-10279
"", // 		-10280
"", // 		-10281
"", // 		-10282
"", // 		-10283
"", // 		-10284
"", // 		-10285
"", // 		-10286
"", // 		-10287
"", // 		-10288
"", // 		-10289
"", // 		-10290
"", // 		-10291
"", // 		-10292
"", // 		-10293
"", // 		-10294
"", // 		-10295
"", // 		-10296
"", // 		-10297
"", // 		-10298
"", // 		-10299
"", // 		-10300
"", // 		-10301
"", // 		-10302
"", // 		-10303
"", // 		-10304
"", // 		-10305
"", // 		-10306
"", // 		-10307
"", // 		-10308
"", // 		-10309
"", // 		-10310
"", // 		-10311
"", // 		-10312
"", // 		-10313
"", // 		-10314
"", // 		-10315
"", // 		-10316
"", // 		-10317
"", // 		-10318
"", // 		-10319
"", // 		-10320
"", // 		-10321
"", // 		-10322
"", // 		-10323
"", // 		-10324
"", // 		-10325
"", // 		-10326
"", // 		-10327
"", // 		-10328
"", // 		-10329
"", // 		-10330
"", // 		-10331
"", // 		-10332
"", // 		-10333
"", // 		-10334
"", // 		-10335
"", // 		-10336
"", // 		-10337
"", // 		-10338
"", // 		-10339
"noConnectError", // 		-10340
"badConnectError", // 		-10341
"multConnectError", // 		-10342
"SCXIConfigError", // 		-10343
"chassisSynchedError", // 		-10344
"chassisMemAllocError", // 		-10345
"badPacketError", // 		-10346
"chassisCommunicationError", // 		-10347
"waitingForReprogError", // 		-10348
"SCXIModuleTypeConflictError", // 		-10349
"CannotDetermineEntryModuleError", // 		-10350
"", // 		-10351
"", // 		-10352
"", // 		-10353
"", // 		-10354
"", // 		-10355
"", // 		-10356
"", // 		-10357
"", // 		-10358
"", // 		-10359
"DSPInitError", // 		-10360
"", // 		-10361
"", // 		-10362
"", // 		-10363
"", // 		-10364
"", // 		-10365
"", // 		-10366
"", // 		-10367
"", // 		-10368
"", // 		-10369
"badScanListError", // 		-10370
"", // 		-10371
"", // 		-10372
"", // 		-10373
"", // 		-10374
"", // 		-10375
"", // 		-10376
"", // 		-10377
"", // 		-10378
"", // 		-10379
"invalidSignalSrcError", // 		-10380
"invalidSignalNameError", // 		-10381
"invalidSrcSpecError", // 		-10382
"invalidSignalDestError", // 		-10383
"", // 		-10384
"", // 		-10385
"", // 		-10386
"", // 		-10387
"", // 		-10388
"", // 		-10389
"routingError", // 		-10390
"pfiBadLineError", // 		-10391
"pfiGPCTRNotRoutedError", // 		-10392
"pfiDefaultLineUndefinedError", // 		-10393
"pfiDoubleRoutingError", // 		-10394
"", // 		-10395
"", // 		-10396
"", // 		-10397
"", // 		-10398
"", // 		-10399
"userOwnedRsrcError", // 		-10400
"unknownDeviceError", // 		-10401
"deviceNotFoundError", // 		-10402
"deviceSupportError", // 		-10403
"noLineAvailError", // 		-10404
"noChanAvailError", // 		-10405
"noGroupAvailError", // 		-10406
"lineBusyError", // 		-10407
"chanBusyError", // 		-10408
"groupBusyError", // 		-10409
"relatedLCGBusyError", // 		-10410
"counterBusyError", // 		-10411
"noGroupAssignError", // 		-10412
"groupAssignError", // 		-10413
"reservedPinError", // 		-10414
"externalMuxSupportError", // 		-10415
"", // 		-10416
"", // 		-10417
"", // 		-10418
"", // 		-10419
"", // 		-10420
"", // 		-10421
"", // 		-10422
"", // 		-10423
"", // 		-10424
"", // 		-10425
"", // 		-10426
"", // 		-10427
"", // 		-10428
"", // 		-10429
"", // 		-10430
"", // 		-10431
"", // 		-10432
"", // 		-10433
"", // 		-10434
"", // 		-10435
"", // 		-10436
"", // 		-10437
"", // 		-10438
"", // 		-10439
"sysOwnedRsrcError", // 		-10440
"memConfigError", // 		-10441
"memDisabledError", // 		-10442
"memAlignmentError", // 		-10443
"memFullError", // 		-10444
"memLockError", // 		-10445
"memPageError", // 		-10446
"memPageLockError", // 		-10447
"stackMemError", // 		-10448
"cacheMemError", // 		-10449
"physicalMemError", // 		-10450
"virtualMemError", // 		-10451
"noIntAvailError", // 		-10452
"intInUseError", // 		-10453
"noDMACError", // 		-10454
"noDMAAvailError", // 		-10455
"DMAInUseError", // 		-10456
"badDMAGroupError", // 		-10457
"diskFullError", // 		-10458
"DLLInterfaceError", // 		-10459
"interfaceInteractionError", // 		-10460
"resourceReservedError", // 		-10461
"resourceNotReservedError", // 		-10462
"mdResourceAlreadyReservedError", // 		-10463
"mdResourceReservedError", // 		-10464
"mdResourceNotReservedError", // 		-10465
"mdResourceAccessKeyError", // 		-10466
"mdResourceNotRegisteredError", // 		-10467
"", // 		-10468
"", // 		-10469
"", // 		-10470
"", // 		-10471
"", // 		-10472
"", // 		-10473
"", // 		-10474
"", // 		-10475
"", // 		-10476
"", // 		-10477
"", // 		-10478
"", // 		-10479
"muxMemFullError", // 		-10480
"bufferNotInterleavedError", // 		-10481
"waveformBufferSizeError", // 		-10482
"", // 		-10483
"", // 		-10484
"", // 		-10485
"", // 		-10486
"", // 		-10487
"", // 		-10488
"", // 		-10489
"", // 		-10490
"", // 		-10491
"", // 		-10492
"", // 		-10493
"", // 		-10494
"", // 		-10495
"", // 		-10496
"", // 		-10497
"", // 		-10498
"", // 		-10499
"", // 		-10500
"", // 		-10501
"", // 		-10502
"", // 		-10503
"", // 		-10504
"", // 		-10505
"", // 		-10506
"", // 		-10507
"", // 		-10508
"", // 		-10509
"", // 		-10510
"", // 		-10511
"", // 		-10512
"", // 		-10513
"", // 		-10514
"", // 		-10515
"", // 		-10516
"", // 		-10517
"", // 		-10518
"", // 		-10519
"", // 		-10520
"", // 		-10521
"", // 		-10522
"", // 		-10523
"", // 		-10524
"", // 		-10525
"", // 		-10526
"", // 		-10527
"", // 		-10528
"", // 		-10529
"", // 		-10530
"", // 		-10531
"", // 		-10532
"", // 		-10533
"", // 		-10534
"", // 		-10535
"", // 		-10536
"", // 		-10537
"", // 		-10538
"", // 		-10539
"SCXIModuleNotSupportedError", // 		-10540
"TRIG1ResourceConflict", // 		-10541
"matrixTerminalBlockError", // 		-10542
"noMatrixTerminalBlockError", // 		-10543
"invalidMatrixTerminalBlockError", // 		-10544
"", // 		-10545
"", // 		-10546
"", // 		-10547
"", // 		-10548
"", // 		-10549
"", // 		-10550
"", // 		-10551
"", // 		-10552
"", // 		-10553
"", // 		-10554
"", // 		-10555
"", // 		-10556
"", // 		-10557
"", // 		-10558
"", // 		-10559
"invalidDSPHandleError", // 		-10560
"DSPDataPathBusyError", // 		-10561
"", // 		-10562
"", // 		-10563
"", // 		-10564
"", // 		-10565
"", // 		-10566
"", // 		-10567
"", // 		-10568
"", // 		-10569
"", // 		-10570
"", // 		-10571
"", // 		-10572
"", // 		-10573
"", // 		-10574
"", // 		-10575
"", // 		-10576
"", // 		-10577
"", // 		-10578
"", // 		-10579
"", // 		-10580
"", // 		-10581
"", // 		-10582
"", // 		-10583
"", // 		-10584
"", // 		-10585
"", // 		-10586
"", // 		-10587
"", // 		-10588
"", // 		-10589
"", // 		-10590
"", // 		-10591
"", // 		-10592
"", // 		-10593
"", // 		-10594
"", // 		-10595
"", // 		-10596
"", // 		-10597
"", // 		-10598
"", // 		-10599
"noSetupError", // 		-10600
"multSetupError", // 		-10601
"noWriteError", // 		-10602
"groupWriteError", // 		-10603
"activeWriteError", // 		-10604
"endWriteError", // 		-10605
"notArmedError", // 		-10606
"armedError", // 		-10607
"noTransferInProgError", // 		-10608
"transferInProgError", // 		-10609
"transferPauseError", // 		-10610
"badDirOnSomeLinesError", // 		-10611
"badLineDirError", // 		-10612
"badChanDirError", // 		-10613
"badGroupDirError", // 		-10614
"masterClkError", // 		-10615
"slaveClkError", // 		-10616
"noClkSrcError", // 		-10617
"badClkSrcError", // 		-10618
"multClkSrcError", // 		-10619
"noTrigError", // 		-10620
"badTrigError", // 		-10621
"preTrigError", // 		-10622
"postTrigError", // 		-10623
"delayTrigError", // 		-10624
"masterTrigError", // 		-10625
"slaveTrigError", // 		-10626
"noTrigDrvError", // 		-10627
"multTrigDrvError", // 		-10628
"invalidOpModeError", // 		-10629
"invalidReadError", // 		-10630
"noInfiniteModeError", // 		-10631
"someInputsIgnoredError", // 		-10632
"invalidRegenModeError", // 		-10633
"noContTransferInProgressError", // 		-10634
"invalidSCXIOpModeError", // 		-10635
"noContWithSynchError", // 		-10636
"bufferAlreadyConfigError", // 		-10637
"badClkDestError", // 		-10638
"", // 		-10639
"", // 		-10640
"", // 		-10641
"", // 		-10642
"", // 		-10643
"", // 		-10644
"", // 		-10645
"", // 		-10646
"", // 		-10647
"", // 		-10648
"", // 		-10649
"", // 		-10650
"", // 		-10651
"", // 		-10652
"", // 		-10653
"", // 		-10654
"", // 		-10655
"", // 		-10656
"", // 		-10657
"", // 		-10658
"", // 		-10659
"", // 		-10660
"", // 		-10661
"", // 		-10662
"", // 		-10663
"", // 		-10664
"", // 		-10665
"", // 		-10666
"", // 		-10667
"", // 		-10668
"", // 		-10669
"rangeBadForMeasModeError", // 		-10670
"autozeroModeConflictError", // 		-10671
"", // 		-10672
"", // 		-10673
"", // 		-10674
"", // 		-10675
"", // 		-10676
"", // 		-10677
"", // 		-10678
"", // 		-10679
"badChanGainError", // 		-10680
"badChanRangeError", // 		-10681
"badChanPolarityError", // 		-10682
"badChanCouplingError", // 		-10683
"badChanInputModeError", // 		-10684
"clkExceedsBrdsMaxConvRateError", // 		-10685
"scanListInvalidError", // 		-10686
"bufferInvalidError", // 		-10687
"noTrigEnabledError", // 		-10688
"digitalTrigBError", // 		-10689
"digitalTrigAandBError", // 		-10690
"extConvRestrictionError", // 		-10691
"chanClockDisabledError", // 		-10692
"extScanClockError", // 		-10693
"unsafeSamplingFreqError", // 		-10694
"DMAnotAllowedError", // 		-10695
"multiRateModeError", // 		-10696
"rateNotSupportedError", // 		-10697
"timebaseConflictError", // 		-10698
"polarityConflictError", // 		-10699
"signalConflictError", // 		-10700
"noLaterUpdateError", // 		-10701
"prePostTriggerError", // 		-10702
"", // 		-10703
"", // 		-10704
"", // 		-10705
"", // 		-10706
"", // 		-10707
"", // 		-10708
"", // 		-10709
"noHandshakeModeError", // 		-10710
"", // 		-10711
"", // 		-10712
"", // 		-10713
"", // 		-10714
"", // 		-10715
"", // 		-10716
"", // 		-10717
"", // 		-10718
"", // 		-10719
"noEventCtrError", // 		-10720
"", // 		-10721
"", // 		-10722
"", // 		-10723
"", // 		-10724
"", // 		-10725
"", // 		-10726
"", // 		-10727
"", // 		-10728
"", // 		-10729
"", // 		-10730
"", // 		-10731
"", // 		-10732
"", // 		-10733
"", // 		-10734
"", // 		-10735
"", // 		-10736
"", // 		-10737
"", // 		-10738
"", // 		-10739
"SCXITrackHoldError", // 		-10740
"", // 		-10741
"", // 		-10742
"", // 		-10743
"", // 		-10744
"", // 		-10745
"", // 		-10746
"", // 		-10747
"", // 		-10748
"", // 		-10749
"", // 		-10750
"", // 		-10751
"", // 		-10752
"", // 		-10753
"", // 		-10754
"", // 		-10755
"", // 		-10756
"", // 		-10757
"", // 		-10758
"", // 		-10759
"", // 		-10760
"", // 		-10761
"", // 		-10762
"", // 		-10763
"", // 		-10764
"", // 		-10765
"", // 		-10766
"", // 		-10767
"", // 		-10768
"", // 		-10769
"", // 		-10770
"", // 		-10771
"", // 		-10772
"", // 		-10773
"", // 		-10774
"", // 		-10775
"", // 		-10776
"", // 		-10777
"", // 		-10778
"", // 		-10779
"sc2040InputModeError", // 		-10780
"outputTypeMustBeVoltageError", // 		-10781
"sc2040HoldModeError", // 		-10782
"calConstPolarityConflictError", // 		-10783
"masterDeviceNotInPXISlot2", // 		-10784
"", // 		-10785
"", // 		-10786
"", // 		-10787
"", // 		-10788
"", // 		-10789
"", // 		-10790
"", // 		-10791
"", // 		-10792
"", // 		-10793
"", // 		-10794
"", // 		-10795
"", // 		-10796
"", // 		-10797
"", // 		-10798
"", // 		-10799
"timeOutError", // 		-10800
"calibrationError", // 		-10801
"dataNotAvailError", // 		-10802
"transferStoppedError", // 		-10803
"earlyStopError", // 		-10804
"overRunError", // 		-10805
"noTrigFoundError", // 		-10806
"earlyTrigError", // 		-10807
"LPTcommunicationError", // 		-10808
"gateSignalError", // 		-10809
"internalDriverError", // 		-10810
"frequencyMeasurementError", // 		-10811
"", // 		-10812
"", // 		-10813
"", // 		-10814
"", // 		-10815
"", // 		-10816
"", // 		-10817
"", // 		-10818
"", // 		-10819
"", // 		-10820
"", // 		-10821
"", // 		-10822
"", // 		-10823
"", // 		-10824
"", // 		-10825
"", // 		-10826
"", // 		-10827
"", // 		-10828
"", // 		-10829
"", // 		-10830
"", // 		-10831
"", // 		-10832
"", // 		-10833
"", // 		-10834
"", // 		-10835
"", // 		-10836
"", // 		-10837
"", // 		-10838
"", // 		-10839
"softwareError", // 		-10840
"firmwareError", // 		-10841
"hardwareError", // 		-10842
"underFlowError", // 		-10843
"underWriteError", // 		-10844
"overFlowError", // 		-10845
"overWriteError", // 		-10846
"dmaChainingError", // 		-10847
"noDMACountAvailError", // 		-10848
"OpenFileError", // 		-10849
"closeFileError", // 		-10850
"fileSeekError", // 		-10851
"readFileError", // 		-10852
"writeFileError", // 		-10853
"miscFileError", // 		-10854
"osUnsupportedError", // 		-10855
"osError", // 		-10856
"internalKernelError", // 		-10857
"hardwareConfigChangedError", // 		-10858
"", // 		-10859
"", // 		-10860
"", // 		-10861
"", // 		-10862
"", // 		-10863
"", // 		-10864
"", // 		-10865
"", // 		-10866
"", // 		-10867
"", // 		-10868
"", // 		-10869
"", // 		-10870
"", // 		-10871
"", // 		-10872
"", // 		-10873
"", // 		-10874
"", // 		-10875
"", // 		-10876
"", // 		-10877
"", // 		-10878
"", // 		-10879
"updateRateChangeError", // 		-10880
"partialTransferCompleteError", // 		-10881
"daqPollDataLossError", // 		-10882
"wfmPollDataLossError", // 		-10883
"pretrigReorderError", // 		-10884
"overLoadError", // 		-10885
"", // 		-10886
"", // 		-10887
"", // 		-10888
"", // 		-10889
"", // 		-10890
"", // 		-10891
"", // 		-10892
"", // 		-10893
"", // 		-10894
"", // 		-10895
"", // 		-10896
"", // 		-10897
"", // 		-10898
"", // 		-10899
"", // 		-10900
"", // 		-10901
"", // 		-10902
"", // 		-10903
"", // 		-10904
"", // 		-10905
"", // 		-10906
"", // 		-10907
"", // 		-10908
"", // 		-10909
"", // 		-10910
"", // 		-10911
"", // 		-10912
"", // 		-10913
"", // 		-10914
"", // 		-10915
"", // 		-10916
"", // 		-10917
"", // 		-10918
"", // 		-10919
"gpctrDataLossError", // 		-10920
"", // 		-10921
"", // 		-10922
"", // 		-10923
"", // 		-10924
"", // 		-10925
"", // 		-10926
"", // 		-10927
"", // 		-10928
"", // 		-10929
"", // 		-10930
"", // 		-10931
"", // 		-10932
"", // 		-10933
"", // 		-10934
"", // 		-10935
"", // 		-10936
"", // 		-10937
"", // 		-10938
"", // 		-10939
"chassisResponseTimeoutError", // 		-10940
"reprogrammingFailedError", // 		-10941
"invalidResetSignatureError", // 		-10942
"chassisLockupError" // 		-10943
};

const char* NIDAQ_Error_Message(i16 code)
{
	if (code==0) return "";
	else if (code<-10000 && code>-10944)
		return NIDAQ_Error[-code-10000];
	else return NIDAQ_Error[0];
}

} // namespace pci6602

