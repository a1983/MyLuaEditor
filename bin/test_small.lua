
-- Responds to UI updates.  This includes moving the cursor.
-- CATEGORY: SciTE event handler
function M.OnUpdateUI()
  -- Disable any autocomplete indicators if cursor moved away.
  if AUTOCOMPLETE_SYNTAX then
    if editor:IndicatorValueAt(INDICATOR_AUTOCOMPLETE, editor.CurrentPos) ~= 1 then
      editor.IndicatorCurrent = INDICATOR_AUTOCOMPLETE
      editor:IndicatorClearRange(0, editor.Length)
    end
  end

  -- This updates the AST when the selection is moved to a different line.
  if not UPDATE_ALWAYS then
    local currentline = editor:LineFromPosition(editor.Anchor)
    if currentline ~= buffer.lastline then
      update_ast()
      buffer.lastline = currentline
    end
  end

  if buffer.src ~= editor:GetText() then return end -- skip if AST is not up-to-date

  -- check if selection if currently on identifier
  local selectedtoken, id = getselectedvariable()

  --test: adding items to context menu upon variable selection
  --if id then
  --  props['user.context.menu'] = selectednote.ast[1] .. '|1101'
  --  --Q: how to reliably remove this upon a buffer switch?
  --end

  -- Highlight all instances of that identifier.
  editor:MarkerDeleteAll(MARKER_SCOPEBEGIN)
  editor:MarkerDeleteAll(MARKER_SCOPEMIDDLE)
  editor:MarkerDeleteAll(MARKER_SCOPEEND)
  editor:MarkerDeleteAll(MARKER_MASKED)
  editor:MarkerDeleteAll(MARKER_MASKING)
  editor.IndicatorCurrent = INDICATOR_SCOPE
  editor:IndicatorClearRange(0, editor.Length)
  editor.IndicatorCurrent = INDICATOR_MASKED
  editor:IndicatorClearRange(0, editor.Length)
  if id then

    -- Indicate uses of variable.
    editor.IndicatorCurrent = INDICATOR_SCOPE
    local ftoken, ltoken -- first and last occurances
    for _,token in ipairs(buffer.tokenlist) do
      if token.ast.id == id then
        ltoken = token
        if not ftoken then ftoken = token end
        editor:IndicatorFillRange(token.fpos-1, token.lpos-token.fpos+1)
      end
    end

    scope_positions(ftoken.fpos-1, ltoken.lpos-1)

    -- identify any local definition masked by any selected local definition.
    local ast = selectedtoken -- cast: `Id tokens are AST nodes.
    if ast.localmasking then
      local fpos, lpos = LA.ast_pos_range(ast.localmasking, buffer.tokenlist)
      if fpos then
        local maskedlinenum0 = editor:LineFromPosition(fpos-1)
        local maskinglinenum0 = editor:LineFromPosition(selectedtoken.fpos-1)
        editor:MarkerAdd(maskedlinenum0, MARKER_MASKED)
        editor:MarkerAdd(maskinglinenum0, MARKER_MASKING)
        editor.IndicatorCurrent = INDICATOR_MASKED
        editor:IndicatorFillRange(fpos-1, lpos-fpos+1)
      end
    end
  end

  -- Highlight related keywords.
  do
    editor.IndicatorCurrent = INDICATOR_KEYWORD
    editor:IndicatorClearRange(0, editor.Length)

    -- Check for selection over statement or expression.
    local fpos, lpos = editor.Anchor, editor.CurrentPos
    if lpos < fpos then fpos, lpos = lpos, fpos end -- swap
    fpos, lpos = fpos + 1, lpos + 1 - 1
    local match1_ast, match1_comment, iswhitespace =
      LA.smallest_ast_containing_range(buffer.ast, buffer.tokenlist, fpos, lpos)
    -- DEBUG('m', match1_ast and match1_ast.tag, match1_comment, iswhitespace)

    -- Find and highlight.
    local keywords; keywords, match1_ast = LI.related_keywords(match1_ast, buffer.ast, buffer.tokenlist, buffer.src)
    if keywords then
      for i=1,#keywords do
        local fpos, lpos = keywords[i].fpos, keywords[i].lpos
        editor:IndicatorFillRange(fpos-1, lpos-fpos+1)
      end
    end

    -- Mark range of lines covered by item on selection.
    if not id then
      local fpos, lpos = LA.ast_pos_range(match1_ast, buffer.tokenlist)
      if fpos then scope_positions(fpos-1, lpos-1) end
    end
  end


  --[[
  -- Display callinfo help on function.
  if selectednote and selectednote.ast.resolvedname and LS.global_signatures[selectednote.ast.resolvedname] then
    local name = selectednote.ast.resolvedname
    editor:CallTipShow(editor.Anchor, LS.global_signatures[name])
  else
    --editor:CallTipCancel()
  end
  ]]
end


-- Responds to requests for restyling.
-- Note: if StartStyling is not applied over the entire requested range, than this function is quickly recalled
--   (which possibly can be useful for incremental updates)
-- CATEGORY: SciTE event handler
local style_delay_count = 0
local isblock = {Function=true}
local debug_recursion = 0
function M.OnStyle(styler)
  assert(styler.language == "script_lua")

  -- Optionally delay styling.
  --print('DEBUG:style-count', style_delay_count)
  if style_delay_count > 0 then
    -- Dislpay wait marker if not displayed and new text parsing not yet attempted.
    if not buffer.wait_marker_line and editor:GetText() ~= buffer.lastsrc then
      buffer.wait_marker_line = editor:LineFromPosition(editor.CurrentPos)
      editor:MarkerDeleteAll(MARKER_WAIT)
      editor:MarkerAdd(buffer.wait_marker_line, MARKER_WAIT)
      style_delay_count = style_delay_count + 1
        -- +1 is hack to work around warning described below.
    end
    style_delay_count = style_delay_count - 1
    return
  elseif style_delay_count == 0 then
    if buffer.wait_marker_line then
      editor:MarkerDeleteAll(MARKER_WAIT)
      buffer.wait_marker_line = nil
    end
  end
  style_delay_count = UPDATE_DELAY
  -- WARNING: updating marker causes another style event to be called immediately.
  -- Therefore, we take care to only update marker when marker state needs changed
  -- and correct the count when we do.

  --IMPROVE: could metalua libraries parse text across multiple calls to
  --`OnStyle` to reduce long pauses with big files?  Maybe use coroutines.

  --DEBUG("style",styler.language, styler.startPos, styler.lengthDoc, styler.initStyle)

  -- update AST if needed
  if UPDATE_ALWAYS then
    update_ast()
  elseif not buffer.lastsrc then
    -- this ensures that AST compiling is attempted when file is first loaded since OnUpdateUI
    -- is not called on load.
    update_ast()
  end

  --DEBUG('OnStyle', editor:LineFromPosition(styler.startPos), editor:LineFromPosition(styler.startPos+styler.lengthDoc), styler.initStyle)
  if buffer.src ~= editor:GetText() then return end  -- skip if AST not up-to-date
  -- WARNING: SciTE will repeatedly call OnStyle until StartStyling is performed.
  -- However, StartStyling/Forward/EndStyling clears styles in the given range,
  -- but we prefer to leave the styles as is.

  debug_recursion = debug_recursion + 1
  if debug_recursion ~= 1 then print('warning: OnStyle recursion', debug_recursion) end
      -- folding previously triggered recursion leading to odd effects; make sure this is gone

  -- Apply SciTE styling
  editor.StyleHotSpot[S_LOCAL] = true
  editor.StyleHotSpot[S_LOCAL_MUTATE] = true
  editor.StyleHotSpot[S_LOCAL_UNUSED] = true
  editor.StyleHotSpot[S_LOCAL_PARAM] = true
  editor.StyleHotSpot[S_LOCAL_PARAM_MUTATE] = true
  editor.StyleHotSpot[S_UPVALUE] = true
  editor.StyleHotSpot[S_UPVALUE_MUTATE] = true
  editor.StyleHotSpot[S_GLOBAL_RECOGNIZED] = true
  editor.StyleHotSpot[S_GLOBAL_UNRECOGNIZED] = true
  editor.StyleHotSpot[S_FIELD] = true
  editor.StyleHotSpot[S_FIELD_RECOGNIZED] = true
  -- note: SCN_HOTSPOTCLICK, SCN_HOTSPOTDOUBLECLICK currently aren't
  -- implemented by SciTE, although it has been proposed.

  local startpos0, endpos0 = 0, editor.Length -1
  styler:StartStyling(startpos0, endpos0 - startpos0 + 1, 0)
  -- local startpos0 = styler.startPos
  --styler:StartStyling(styler.startPos, styler.lengthDoc, styler.initStyle)
  --   a partial range like this doesn't work right since variables outside of edited range
  --   may need styling adjusted (e.g. a local variable definition that becomes unused)

  local i=startpos0+1
  local tokenidx = 1
  local token = buffer.tokenlist[tokenidx]
  local function nexttoken() tokenidx = tokenidx+1; token = buffer.tokenlist[tokenidx] end
  while styler:More() do
    while token and i > token.lpos do
      nexttoken()
    end

    if token and i >= token.fpos and i <= token.lpos then
      local ast = token.ast
      if token.tag == 'Id' then
        if ast.localdefinition then -- local
          if not ast.localdefinition.isused then
            styler:SetState(S_LOCAL_UNUSED)
          elseif ast.localdefinition.functionlevel  < ast.functionlevel then  -- upvalue
            if ast.localdefinition.isset then
              styler:SetState(S_UPVALUE_MUTATE)
            else
              styler:SetState(S_UPVALUE)
            end
          elseif ast.localdefinition.isparam then
            if ast.localdefinition.isset then
              styler:SetState(S_LOCAL_PARAM_MUTATE)
            else
              styler:SetState(S_LOCAL_PARAM)
            end
          else
            if ast.localdefinition.isset then
              styler:SetState(S_LOCAL_MUTATE)
            else
              styler:SetState(S_LOCAL)
            end
          end
        else -- global
          if ast.definedglobal then
            styler:SetState(S_GLOBAL_RECOGNIZED)
          else
            styler:SetState(S_GLOBAL_UNRECOGNIZED)
          end
        end
      elseif ast.isfield then -- implies token.tag == 'String'
        local val = ast.seevalue.value
        if ast.definedglobal or val ~= T.universal and not T.iserror[val] and val ~= nil then
          styler:SetState(S_FIELD_RECOGNIZED)
        else
          styler:SetState(S_FIELD)
        end
      elseif token.tag == 'Comment' then
        styler:SetState(S_COMMENT)
      elseif token.tag == 'String' then -- note: excludes ast.isfield
        styler:SetState(S_STRING)
      elseif token.tag == 'Keyword' then
        styler:SetState(S_KEYWORD)
      else
        styler:SetState(S_DEFAULT)
      end
    elseif styler:Current() == '\t' then
      styler:SetState(S_TAB)
    else
      styler:SetState(S_DEFAULT)
    end
    styler:Forward()
    i = i + #styler:Current()  -- support Unicode
  end
  styler:EndStyling()

  -- Apply indicators in token list.
  -- Mark masking local variables and warnings.
  editor.IndicatorCurrent = INDICATOR_MASKING
  editor:IndicatorClearRange(0, editor.Length)
  editor.IndicatorCurrent = INDICATOR_WARNING
  editor:IndicatorClearRange(0, editor.Length)
  editor.IndicatorCurrent = INDICATOR_DEADCODE
  editor:IndicatorClearRange(0, editor.Length)
  local tokenlist = buffer.tokenlist
  for idx=1,#tokenlist do
    local token = tokenlist[idx]
    local ast = token.ast
    if ast and ast.localmasking then
      editor.IndicatorCurrent = INDICATOR_MASKING
      editor:IndicatorFillRange(token.fpos-1, token.lpos - token.fpos + 1)
    end
    if ast and (ast.seevalue or ast).note then
      local hast = ast.seevalue or ast
      if hast.tag == 'Call' then hast = hast[1] elseif hast.tag == 'Invoke' then hast = hast[2] end
        -- note: for calls only highlight function name
      local fpos, lpos = LA.ast_pos_range(hast, buffer.tokenlist)
      editor.IndicatorCurrent = INDICATOR_WARNING
      editor:IndicatorFillRange(fpos-1, lpos-fpos+1)
    end
    if ast and ast.isdead then
      local fpos, lpos = LA.ast_pos_range(ast, buffer.tokenlist)
      editor.IndicatorCurrent = INDICATOR_DEADCODE
      editor:IndicatorFillRange(fpos-1, lpos-fpos+1)
    end
  end

  -- Apply folding.
  if ENABLE_FOLDING then
    clockbegin 'f1'
    local fsline1 = editor:LineFromPosition(startpos0)+1
    local lsline1 = editor:LineFromPosition(endpos0)+1
    --print('DEBUG:+', linea0,lineb0) -- test for recursion
    -- IMPROVE: This might be done only over styler.startPos, styler.lengthDoc.
    --   Does that improve performance?
    local level = 0
    local levels = {}
    local plinenum1 = 1
    local firstseen = {}
    for _, token in ipairs(buffer.tokenlist) do
      -- Fill line numbers up to and including this token.
      local llinenum1 = editor:LineFromPosition(token.lpos-1)+1
          -- note: much faster than non-caching LA.pos_to_linecol.
      for linenum1=plinenum1,llinenum1 do levels[linenum1] = levels[linenum1] or level end

      -- Monitor level changes and set any header flags.
      if token.ast and token.ast.tag == 'Function' then
        if not firstseen[token.ast] then
           level = level + 1
           firstseen[token.ast] = llinenum1
        elseif token[1] == 'end' then
          level = level -1
          local beginlinenum1 = firstseen[token.ast]
          if llinenum1 > beginlinenum1 then
            local old_value = levels[beginlinenum1]
            if old_value < SC_FOLDLEVELHEADERFLAG then
             levels[beginlinenum1] = old_value + SC_FOLDLEVELHEADERFLAG
            end
          end
        end
      end -- careful: in Metalua, `function` is not always part of the `Function node.

      plinenum1 = llinenum1 + 1
    end
    for line1=plinenum1,editor.LineCount do levels[line1] = level end -- fill remaining
    --for line1=1,#levels do print('DEBUG:', line1, levels[line1]) end
    for line1=1,#levels do -- apply
    --for line1=fsline1,lsline1 do -- apply
      styler:SetLevelAt(line1-1, levels[line1])
        --Q:why does this seem to sometimes trigger recursive OnStyle calls? (see below).
    end
    clockend 'f2'
    -- Caution: careful folding if StartStyling is performed over a range larger
    --   than suggested by startPos/lengthDoc.
    -- Note: Folding sometimes tend to trigger OnStyle recursion, leading to odd problems.  This
    --   seems reduced now but not gone (e.g. load types.lua).
    --   The following old comments are left here:
    -- #  Changing a flag on a line more than once triggers heavy recursion, even stack overflow:
    -- #     styler:SetLevelAt(0,1)
    -- #     styler:SetLevelAt(0,1 + SC_FOLDLEVELHEADERFLAG)
    -- #  Setting levels only on lines being styled may reduce though not eliminate recursion.
    -- #  Iterating in reverse may reduce though not eliminate recursion.
    -- #  Disabling folding completely eliminates recursion.
    --print'DEBUG:-'  -- test for recursion
  end

  debug_recursion = debug_recursion - 1
end


-- CATEGORY: SciTE event handler
function M.OnDoubleClick()
  if buffer.src ~= editor:GetText() then return end -- skip if AST is not up-to-date

  -- check if selection if currently on identifier
  local token = getselectedvariable()
  if token and token.ast then
    local info  = LI.get_value_details(token.ast, buffer.tokenlist, buffer.src)
    editor:CallTipShow(token.fpos-1, info)
  end
end