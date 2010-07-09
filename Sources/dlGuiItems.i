#ifdef LabCurves_GUI_INPUT_ITEM
// Here comes the description of the numerical input elements.
// Attention : Default,Min,Max,Step should be consistent int or double. Double *always* in X.Y notation to indicate so.
// Unique Name,GuiElement,InitLevel,InJobFile,HasDefault (causes button too !),Default,Min,Max,Step,NrDecimals,Label,ToolTip
#endif

#ifdef LabCurves_GUI_CHOICE_ITEM
// Here comes the description of the choice elements.
// Unique Name,GuiElement,InitLevel,InJobFile,HasDefault (causes button too !),Default,Choices (from dlGuiOptions.h),ToolTip
{"PipeSize"                    ,dlGT_Choice       ,2,0,1 ,dlPipeSize_Quarter          ,GuiOptions->PipeSize                  ,_("Size of image processed vs original.")},
{"CurveL"                      ,dlGT_Choice       ,9,1,1 ,dlCurveChoice_None          ,GuiOptions->CurveL                    ,_("L curve")},
{"CurveLa"                     ,dlGT_Choice       ,9,1,1 ,dlCurveChoice_None          ,GuiOptions->Curvea                    ,_("a curve")},
{"CurveLb"                     ,dlGT_Choice       ,9,1,1 ,dlCurveChoice_None          ,GuiOptions->Curveb                    ,_("b curve")},
{"CurveSaturation"             ,dlGT_Choice       ,9,1,1 ,dlCurveChoice_None          ,GuiOptions->CurveSaturation           ,_("Saturation curve")},
{"ViewLAB"                     ,dlGT_Choice       ,2,1,1 ,dlViewLAB_LAB               ,GuiOptions->ViewLAB              ,_("View seperate LAB channels")},
#endif

#ifdef LabCurves_GUI_CHECK_ITEM
// Name, GuiType,InitLevel,InJobFile,Default,Label,Tip
{"RunMode"                    ,dlGT_Check ,1,0,0,_("manual")          ,_("manual or automatic pipe")},
#endif
