/*******************************************************************************
 *
 * (c) Copyright IBM Corp. 2000, 2016
 *
 *  This program and the accompanying materials are made available
 *  under the terms of the Eclipse Public License v1.0 and
 *  Apache License v2.0 which accompanies this distribution.
 *
 *      The Eclipse Public License is available at
 *      http://www.eclipse.org/legal/epl-v10.html
 *
 *      The Apache License v2.0 is available at
 *      http://www.opensource.org/licenses/apache2.0.php
 *
 * Contributors:
 *    Multiple authors (IBM Corp.) - initial implementation and documentation
 *******************************************************************************/

#include "codegen/Relocation.hpp"

#include <stddef.h>                         // for NULL
#include <stdint.h>                         // for uint8_t, uintptr_t, etc
#include "codegen/AheadOfTimeCompile.hpp"   // for AheadOfTimeCompile
#include "codegen/CodeGenerator.hpp"        // for CodeGenerator
#include "codegen/Instruction.hpp"          // for Instruction
#include "codegen/Linkage.hpp"              // for Linkage
#include "compile/Compilation.hpp"          // for Compilation
#include "control/Options.hpp"
#include "control/Options_inlines.hpp"
#include "env/CompilerEnv.hpp"
#include "env/TRMemory.hpp"                 // for TR_Link::operator new
#include "env/jittypes.h"                   // for intptrj_t
#include "il/symbol/LabelSymbol.hpp"        // for LabelSymbol
#include "infra/Assert.hpp"                 // for TR_ASSERT
#include "infra/Flags.hpp"                  // for flags8_t
#include "infra/Link.hpp"                   // for TR_LinkHead, TR_Link
#include "runtime/Runtime.hpp"

void TR::Relocation::apply(TR::CodeGenerator *codeGen)
   {
   TR_ASSERT(0, "Should never get here");
   }

void TR::Relocation::trace(TR::Compilation* comp)
   {
   TR_ASSERT(0, "Should never get here");
   }

void TR::Relocation::setDebugInfo(TR::RelocationDebugInfo* info)
   {
   this->_genData= info;
   }

TR::RelocationDebugInfo* TR::Relocation::getDebugInfo()
   {
   return this->_genData;
   }
void TR::LabelRelative8BitRelocation::apply(TR::CodeGenerator *codeGen)
   {
   // *this    swipeable for debugging purposes
   AOTcgDiag2(codeGen->comp(), "TR::LabelRelative8BitRelocation::apply cursor=%x label=%x\n", getUpdateLocation(), getLabel());
   codeGen->apply8BitLabelRelativeRelocation((int32_t *)getUpdateLocation(), getLabel());
   }

void TR::LabelRelative12BitRelocation::apply(TR::CodeGenerator *codeGen)
   {
   // *this    swipeable for debugging purposes
   AOTcgDiag2(codeGen->comp(), "TR::LabelRelative12BitRelocation::apply cursor=%x label=%x\n", getUpdateLocation(), getLabel());
   codeGen->apply12BitLabelRelativeRelocation((int32_t *)getUpdateLocation(), getLabel(), isCheckDisp());
   }

void TR::LabelRelative16BitRelocation::apply(TR::CodeGenerator *codeGen)
   {
   // *this    swipeable for debugging purposes
   AOTcgDiag2(codeGen->comp(), "TR::LabelRelative16BitRelocation::apply cursor=%x label=%x\n", getUpdateLocation(), getLabel());
   if(getAddressDifferenceDivisor() == 1)
   codeGen->apply16BitLabelRelativeRelocation((int32_t *)getUpdateLocation(), getLabel());
   else
     codeGen->apply16BitLabelRelativeRelocation((int32_t *)getUpdateLocation(), getLabel(), getAddressDifferenceDivisor(), isInstructionOffset());
   }

void TR::LabelRelative24BitRelocation::apply(TR::CodeGenerator *codeGen)
   {
   // *this    swipeable for debugging purposes
   AOTcgDiag2(codeGen->comp(), "TR::LabelRelative24BitRelocation::apply cursor=%x label=%x\n", getUpdateLocation(), getLabel());
   codeGen->apply24BitLabelRelativeRelocation((int32_t *)getUpdateLocation(), getLabel());
   }

void TR::LabelRelative32BitRelocation::apply(TR::CodeGenerator *codeGen)
   {
   // *this    swipeable for debugging purposes
   AOTcgDiag2(codeGen->comp(), "TR::LabelRelative32BitRelocation::apply cursor=%x label=%x\n", getUpdateLocation(), getLabel());
   codeGen->apply32BitLabelRelativeRelocation((int32_t *)getUpdateLocation(), getLabel());
   }

void TR::LabelAbsoluteRelocation::apply(TR::CodeGenerator *codeGen)
   {
   // *this    swipeable for debugging purposes
   intptrj_t *cursor = (intptrj_t *)getUpdateLocation();
   AOTcgDiag2(codeGen->comp(), "TR::LabelAbsoluteRelocation::apply cursor=%x label=%x\n", cursor, getLabel());
   *cursor = (intptrj_t)getLabel()->getCodeLocation();
   }

void TR::InstructionAbsoluteRelocation::apply(TR::CodeGenerator *codeGen)
   {
   intptrj_t *cursor = (intptrj_t*)getUpdateLocation();
   intptrj_t address = (intptrj_t)getInstruction()->getBinaryEncoding();
   if (useEndAddress())
      address += getInstruction()->getBinaryLength();
   AOTcgDiag2(codeGen->comp(), "TR::InstructionAbsoluteRelocation::apply cursor=%x instruction=%x\n", cursor, address);
   *cursor = address;
   }


void TR::LoadLabelRelative16BitRelocation::apply(TR::CodeGenerator *codeGen)
   {
   AOTcgDiag3(codeGen->comp(), "TR::LoadLabelRelative16BitRelocation::apply lastInstruction=%x startLabel=%x endLabel=%x\n", getLastInstruction(), getStartLabel(), getEndLabel());
   codeGen->apply16BitLoadLabelRelativeRelocation(getLastInstruction(), getStartLabel(), getEndLabel(), getDeltaToStartLabel());
   }

void TR::LoadLabelRelative32BitRelocation::apply(TR::CodeGenerator *codeGen)
   {
   AOTcgDiag3(codeGen->comp(), "TR::LoadLabelRelative32BitRelocation::apply lastInstruction=%x startLabel=%x endLabel=%x\n", getLastInstruction(), getStartLabel(), getEndLabel());
   codeGen->apply32BitLoadLabelRelativeRelocation(getLastInstruction(), getStartLabel(), getEndLabel(), getDeltaToStartLabel());
   }

void TR::LoadLabelRelative64BitRelocation::apply(TR::CodeGenerator *codeGen)
   {
   AOTcgDiag2(codeGen->comp(), "TR::LoadLabelRelative64BitRelocation::apply lastInstruction=%x label=%x\n", getLastInstruction(), getLabel());
   codeGen->apply64BitLoadLabelRelativeRelocation(getLastInstruction(), getLabel());
   }

uint8_t TR::ExternalRelocation::collectModifier()
   {
   // *this    swipeable for debugging purposes
   TR::Compilation *comp = TR::comp();
   uint8_t * aotMethodCodeStart = (uint8_t *)comp->getAotMethodCodeStart();
   uint8_t * updateLocation;

   if (TR::Compiler->target.cpu.isPower() &&
          (_kind == TR_ArrayCopyHelper || _kind == TR_ArrayCopyToc || _kind == TR_RamMethod || _kind == TR_GlobalValue || _kind == TR_BodyInfoAddressLoad || _kind == TR_DataAddress || _kind == TR_JNISpecialTargetAddress || _kind == TR_JNIStaticTargetAddress || _kind == TR_JNIVirtualTargetAddress
                || _kind == TR_StaticRamMethodConst || _kind == TR_VirtualRamMethodConst || _kind == TR_SpecialRamMethodConst))
      {
      TR::Instruction *instr = (TR::Instruction *)getUpdateLocation();
      updateLocation = instr->getBinaryEncoding();
      }
   else
      {
      updateLocation = getUpdateLocation();
      }

   int32_t distance = updateLocation - aotMethodCodeStart;
   AOTcgDiag1(comp, "TR::ExternalRelocation::collectModifier distance=%x\n", distance);

   if (distance < MIN_SHORT_OFFSET || distance > MAX_SHORT_OFFSET)
      return RELOCATION_TYPE_WIDE_OFFSET;

   return 0;
   }

void TR::ExternalRelocation::addAOTRelocation(TR::CodeGenerator *codeGen)
   {
   // *this    swipeable for debugging purposes
   TR::Compilation *comp = codeGen->comp();
   AOTcgDiag0(comp, "TR::ExternalRelocation::addAOTRelocation\n");
   if (comp->getOption(TR_AOT))
      {
      TR_LinkHead<TR::IteratedExternalRelocation>& aot = codeGen->getAheadOfTimeCompile()->getAOTRelocationTargets();
      uint32_t narrowSize = getNarrowSize();
      uint32_t wideSize = getWideSize();
      flags8_t modifier(collectModifier());
      TR::IteratedExternalRelocation *r;

      AOTcgDiag1(comp, "target=%x\n", _targetAddress);
      if (_targetAddress2)
         AOTcgDiag1(comp, "target2=%x\n", _targetAddress2);
      for (r = aot.getFirst();
           r != 0;
           r = r->getNext())
         {
         if (r->getTargetAddress2())
            AOTcgDiag6(comp, "r=%x full=%x target=%x target2=%x, kind=%x modifier=%x\n",
               r, r->full(), r->getTargetAddress(), r->getTargetAddress2(), r->getTargetKind(), r->getModifierValue());
         else
            AOTcgDiag5(comp, "r=%x full=%x target=%x kind=%x modifier=%x\n",
               r, r->full(), r->getTargetAddress(), r->getTargetKind(), r->getModifierValue());
         AOTcgDiag2(comp, "#sites=%x size=%x\n", r->getNumberOfRelocationSites(), r->getSizeOfRelocationData());

         if (r->full() == false                        &&
             r->getTargetAddress()  == _targetAddress  &&
             r->getTargetAddress2() == _targetAddress2 &&
             r->getTargetKind() == _kind               &&
             modifier.getValue() == r->getModifierValue())
            {
            if (!r->needsWideOffsets())
               {
               if (r->getSizeOfRelocationData() + narrowSize
                   > MAX_SIZE_RELOCATION_DATA)
                  {
                  r->setFull();
                  continue;  // look for one that's not full
                  }
               }
            else
               {
               if (r->getSizeOfRelocationData() + wideSize
                   > MAX_SIZE_RELOCATION_DATA)
                  {
                  r->setFull();
                  continue;  // look for one that's not full
                  }
               }
            r->setNumberOfRelocationSites(r->getNumberOfRelocationSites()+1);
            r->setSizeOfRelocationData(r->getSizeOfRelocationData() +
                               (r->needsWideOffsets()?wideSize:narrowSize));
            _relocationRecord = r;
            if (r->getTargetAddress2())
               AOTcgDiag6(comp, "r=%x full=%x target=%x target2=%x kind=%x modifier=%x\n",
                  r, r->full(), r->getTargetAddress(), r->getTargetAddress2(), r->getTargetKind(), r->getModifierValue());
            else
               AOTcgDiag5(comp, "r=%x full=%x target=%x kind=%x modifier=%x\n",
                  r, r->full(), r->getTargetAddress(), r->getTargetKind(), r->getModifierValue());
            AOTcgDiag2(comp, "#sites=%x size=%x\n", r->getNumberOfRelocationSites(), r->getSizeOfRelocationData());
            return;
            }
         }
      TR::IteratedExternalRelocation *temp =   _targetAddress2 ?
         new (codeGen->trHeapMemory()) TR::IteratedExternalRelocation(_targetAddress, _targetAddress2, _kind, modifier, codeGen) :
         new (codeGen->trHeapMemory()) TR::IteratedExternalRelocation(_targetAddress, _kind, modifier, codeGen);

      aot.add(temp);
      if (_targetAddress2)
         AOTcgDiag6(comp, "temp=%x full=%x target=%x target2=%x kind=%x modifier=%x\n",
            temp, temp->full(), temp->getTargetAddress(), temp->getTargetAddress2(), temp->getTargetKind(), temp->getModifierValue());
      else
         AOTcgDiag5(comp, "temp=%x full=%x target=%x kind=%x modifier=%x\n",
            temp, temp->full(), temp->getTargetAddress(), temp->getTargetKind(), temp->getModifierValue());
      AOTcgDiag2(comp, "#sites=%x size=%x\n", temp->getNumberOfRelocationSites(), temp->getSizeOfRelocationData());
      temp->setNumberOfRelocationSites(temp->getNumberOfRelocationSites()+1);
      temp->setSizeOfRelocationData(temp->getSizeOfRelocationData() +
                              (temp->needsWideOffsets()?wideSize:narrowSize));
      _relocationRecord = temp;
      if (_targetAddress2)
         AOTcgDiag6(comp, "temp=%x full=%x target=%x target2=%x kind=%x modifier=%x\n",
            temp, temp->full(), temp->getTargetAddress(), temp->getTargetAddress2(), temp->getTargetKind(), temp->getModifierValue());
      else
         AOTcgDiag5(comp, "temp=%x full=%x target=%x kind=%x modifier=%x\n",
            temp, temp->full(), temp->getTargetAddress(), temp->getTargetKind(), temp->getModifierValue());
      AOTcgDiag2(comp, "#sites=%x size=%x\n", temp->getNumberOfRelocationSites(), temp->getSizeOfRelocationData());
      }
   }

void TR::ExternalRelocation::apply(TR::CodeGenerator *codeGen)
   {
   // *this    swipeable for debugging purposes
   TR::Compilation *comp = codeGen->comp();
   AOTcgDiag1(comp, "TR::ExternalRelocation::apply updateLocation=%x \n", getUpdateLocation());
   if (comp->getOption(TR_AOT))
      {
      uint8_t * aotMethodCodeStart = (uint8_t *)comp->getAotMethodCodeStart();

      getRelocationRecord()->addRelocationEntry((uint32_t)(getUpdateLocation() -
                                                       aotMethodCodeStart));
      }
   }

void TR::ExternalRelocation::trace(TR::Compilation* comp)
   {
   TR::RelocationDebugInfo* data = this->getDebugInfo();
   uint8_t* updateLocation;
   TR_ExternalRelocationTargetKind kind = getRelocationRecord()->getTargetKind();

   updateLocation = getUpdateLocation();
   uint8_t* aotMethodCodeStart = (uint8_t*)comp->getAotMethodCodeStart();
   uint8_t* codeStart = comp->cg()->getCodeStart();
   uintptr_t methodOffset = updateLocation - aotMethodCodeStart;
   uintptr_t programOffset = updateLocation - codeStart;

   if (data)
      {
      traceMsg(comp, "%-35s %-32s %5d      %04x       %04x %8p\n",
       getName(this->getTargetKind()),
       data->file,
       data->line,
       methodOffset,
       programOffset,
       data->node);
      traceMsg(comp, "TargetAddress1:%x,  TargetAddress2:%x\n", this->getTargetAddress(), this->getTargetAddress2());
      }
   }

uint8_t* TR::BeforeBinaryEncodingExternalRelocation::getUpdateLocation()
   {
   if (NULL == TR::ExternalRelocation::getUpdateLocation())
      setUpdateLocation(getUpdateInstruction()->getBinaryEncoding());
   return TR::ExternalRelocation::getUpdateLocation();
   }

TR::ExternalOrderedPair32BitRelocation::ExternalOrderedPair32BitRelocation(
                 uint8_t                         *location1,
                 uint8_t                         *location2,
                 uint8_t                         *target,
                 TR_ExternalRelocationTargetKind  k,
                 TR::CodeGenerator                *codeGen) :
   TR::ExternalRelocation(), _update2Location(location2)
   {
   AOTcgDiag0(codeGen->comp(), "TR::ExternalOrderedPair32BitRelocation::ExternalOrderedPair32BitRelocation\n");
   setUpdateLocation(location1);
   setTargetAddress(target);
   setTargetKind(k);
   }


uint8_t TR::ExternalOrderedPair32BitRelocation::collectModifier()
   {
   TR::Compilation *comp = TR::comp();
   uint8_t * aotMethodCodeStart = (uint8_t *)comp->getAotMethodCodeStart();
   uint8_t * updateLocation;
   uint8_t * updateLocation2;
   TR_ExternalRelocationTargetKind kind = getTargetKind();

   if (TR::Compiler->target.cpu.isPower() &&
          (kind == TR_ArrayCopyHelper || kind == TR_ArrayCopyToc || kind == TR_RamMethod || kind == TR_GlobalValue || kind == TR_BodyInfoAddressLoad || kind == TR_DataAddress))
      {
      TR::Instruction *instr = (TR::Instruction *)getUpdateLocation();
      TR::Instruction *instr2 = (TR::Instruction *)getLocation2();
      updateLocation = instr->getBinaryEncoding();
      updateLocation2 = instr2->getBinaryEncoding();
      }
   else
      {
      updateLocation = getUpdateLocation();
      updateLocation2 = getLocation2();
      }

   int32_t iLoc = updateLocation - aotMethodCodeStart;
   int32_t iLoc2 = updateLocation2 - aotMethodCodeStart;
   AOTcgDiag0(comp, "TR::ExternalOrderedPair32BitRelocation::collectModifier\n");
   if ( (iLoc < MIN_SHORT_OFFSET  || iLoc > MAX_SHORT_OFFSET ) || (iLoc2 < MIN_SHORT_OFFSET || iLoc2 > MAX_SHORT_OFFSET ) )
      return RELOCATION_TYPE_WIDE_OFFSET | RELOCATION_TYPE_ORDERED_PAIR;

   return RELOCATION_TYPE_ORDERED_PAIR;
   }


void TR::ExternalOrderedPair32BitRelocation::apply(TR::CodeGenerator *codeGen)
   {
   TR::Compilation *comp = codeGen->comp();
   AOTcgDiag0(comp, "TR::ExternalOrderedPair32BitRelocation::apply\n");
   if (comp->getOption(TR_AOT))
      {
      TR::IteratedExternalRelocation *rec = getRelocationRecord();
      uint8_t *codeStart = (uint8_t *)comp->getAotMethodCodeStart();
      TR_ExternalRelocationTargetKind kind = getRelocationRecord()->getTargetKind();
      if (TR::Compiler->target.cpu.isPower() &&
          (kind == TR_ArrayCopyHelper || kind == TR_ArrayCopyToc || kind == TR_RamMethodSequence || kind == TR_GlobalValue || kind == TR_BodyInfoAddressLoad || kind == TR_DataAddress))
         {
         TR::Instruction *instr = (TR::Instruction *)getUpdateLocation();
         TR::Instruction *instr2 = (TR::Instruction *)getLocation2();
         rec->addRelocationEntry((uint32_t)(instr->getBinaryEncoding() - codeStart));
         rec->addRelocationEntry((uint32_t)(instr2->getBinaryEncoding() - codeStart));
         }
      else
         {
         rec->addRelocationEntry(getUpdateLocation() - codeStart);
         rec->addRelocationEntry(getLocation2() - codeStart);
         }
      }
   }

// remember to update the debug extensions when add or changing relocations.
char *TR::ExternalRelocation::_externalRelocationTargetKindNames[TR_NumExternalRelocationKinds] =
   {
   "TR_ConstantPool (0)",
   "TR_HelperAddress (1)",
   "TR_RelativeMethodAddress (2)",
   "TR_AbsoluteMethodAddress (3)",
   "TR_DataAddress (4)",
   "TR_ClassObject (5)",
   "TR_MethodObject (6)",
   "TR_InterfaceObject (7)",
   "TR_AbsoluteHelperAddress (8)",
   "TR_FixedSequenceAddress (9)",
   "TR_FixedSequenceAddress2 (10)",
   "TR_JNIVirtualTargetAddress (11)",
   "TR_JNIStaticTargetAddress  (12)",
   "TR_ArrayCopyHelper (13)",
   "TR_ArrayCopyToc (14)",
   "TR_BodyInfoAddress (15)",
   "TR_Thunks (16)",
   "TR_StaticRamMethodConst (17)",
   "TR_Trampolines (18)",
   "TR_PicTrampolines (19)",
   "TR_CheckMethodEnter(20)",
   "TR_RamMethod (21)",
   "TR_RamMethodSequence (22)",
   "TR_RamMethodSequenceReg (23)",
   "TR_VerifyClassObjectForAlloc (24)",
   "TR_ConstantPoolOrderedPair (25)",
   "TR_AbsoluteMethodAddressOrderedPair (36)",
   "TR_VerifyRefArrayForAlloc (27)",
   "TR_J2IThunks (28)",
   "TR_GlobalValue (29)",
   "TR_BodyInfoAddressLoad (30)",
   "TR_ValidateInstanceField (31)",
   "TR_InlinedStaticMethodWithNopGuard (32)",
   "TR_InlinedSpecialMethodWithNopGuard (33)",
   "TR_InlinedVirtualMethodWithNopGuard (34)",
   "TR_InlinedInterfaceMethodWithNopGuard (35)",
   "TR_SpecialRamMethodConst (36)",
   "TR_InlinedHCRMethod (37)",
   "TR_ValidateStaticField (38)",
   "TR_ValidateClass (39)",
   "TR_ClassAddress (40)",
   "TR_HCR (41)",
   "TR_ProfiledMethodGuardRelocation (42)",
   "TR_ProfiledClassGuardRelocation (43)",
   "TR_HierarchyGuardRelocation (44)",
   "TR_AbstractGuardRelocation (45)",
   "TR_ProfiledInlinedMethodRelocation (46)",
   "TR_MethodPointer (47)",
   "TR_ClassPointer (48)",
   "TR_CheckMethodExit (49)",
   "TR_ValidateArbitraryClass (50)",
   "TR_EmitClass (51)",
   "TR_JNISpecialTargetAddress (52)",
   "TR_VirtualRamMethodConst (53)"
   "TR_InlinedInterfaceMethod (54)",
   "TR_InlinedVirtualMethod (55)",
   };

uintptr_t TR::ExternalRelocation::_globalValueList[TR_NumGlobalValueItems] =
   {
   0,          // not used
   0,          // TR_CountForRecompile
   0,          // TR_HeapBase
   0,          // TR_HeapTop
   0,          // TR_HeapBaseForBarrierRange0
   0,          // TR_ActiveCardTableBase
   0           // TR_HeapSizeForBarrierRange0
   };

char *TR::ExternalRelocation::_globalValueNames[TR_NumGlobalValueItems] =
   {
   "not used (0)",
   "TR_CountForRecompile (1)",
   "TR_HeapBase (2)",
   "TR_HeapTop (3)",
   "TR_HeapBaseForBarrierRange0 (4)",
   "TR_ActiveCardTableBase (5)",
   "TR_HeapSizeForBarrierRange0 (6)"
   };


TR::IteratedExternalRelocation::IteratedExternalRelocation(uint8_t *target, TR_ExternalRelocationTargetKind k, flags8_t modifier, TR::CodeGenerator *codeGen)
      : TR_Link<TR::IteratedExternalRelocation>(),
        _numberOfRelocationSites(0),
        _targetAddress(target),
        _targetAddress2(NULL),
        _relocationData(NULL),
        _relocationDataCursor(NULL),
        // initial size is size of header for this type
        _sizeOfRelocationData(codeGen->getAheadOfTimeCompile()->getSizeOfAOTRelocationHeader(k)),
        _recordModifier(modifier.getValue()),
        _full(false),
        _kind(k)
      {
      AOTcgDiag0(TR::comp(), "TR::IteratedExternalRelocation::IteratedExternalRelocation\n");
      }

TR::IteratedExternalRelocation::IteratedExternalRelocation(uint8_t *target, uint8_t *target2, TR_ExternalRelocationTargetKind k, flags8_t modifier, TR::CodeGenerator *codeGen)
      : TR_Link<TR::IteratedExternalRelocation>(),
        _numberOfRelocationSites(0),
        _targetAddress(target),
        _targetAddress2(target2),
        _relocationData(NULL),
        _relocationDataCursor(NULL),
        // initial size is size of header for this type
        _sizeOfRelocationData(codeGen->getAheadOfTimeCompile()->getSizeOfAOTRelocationHeader(k)),
        _recordModifier(modifier.getValue()),
        _full(false),
        _kind(k)
      {
      AOTcgDiag0(TR::comp(), "TR::IteratedExternalRelocation::IteratedExternalRelocation\n");
      }

void TR::IteratedExternalRelocation::initialiseRelocation(TR::CodeGenerator *codeGen)
   {
   // *this    swipeable for debugging purposes
   AOTcgDiag0(TR::comp(), "TR::IteratedExternalRelocation::initialiseRelocation\n");
   _relocationDataCursor = codeGen->getAheadOfTimeCompile()->initialiseAOTRelocationHeader(this);
   }

void TR::IteratedExternalRelocation::addRelocationEntry(uint32_t locationOffset)
   {
   // *this    swipeable for debugging purposes
   TR::Compilation *comp = TR::comp();
   AOTcgDiag1(comp, "TR::IteratedExternalRelocation::addRelocationEntry _relocationDataCursor=%x\n", _relocationDataCursor);
   if (!needsWideOffsets())
      {
      *(uint16_t *)_relocationDataCursor = (uint16_t)locationOffset;
      _relocationDataCursor += 2;
      }
   else
      {
      *(uint32_t *)_relocationDataCursor = locationOffset;
      _relocationDataCursor += 4;
      }
   }

void TR::LabelTable32BitRelocation::apply(TR::CodeGenerator *codeGen)
   {
   codeGen->apply32BitLabelTableRelocation((int32_t *)getUpdateLocation(), getLabel());
   }


