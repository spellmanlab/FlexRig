function [behaviorVars]=importBehavior(session)
if iscell(session)
    sesh=cell2mat(session(1));
else sesh=session;
end
behaviorVars=struct;
T=fileread(sesh);
findMarkers=strfind(T, 'n	');
%@amitch 03-11-24 Modified to accomodate tab-justified tables
% findMarkers(find(findMarkers>500))=[];
% %@amitch attempt at solution:
% if(isempty(findMarkers))
%     findMarkers = strfind(T,'trial'); %note the lack of space
%     findMarkers = findMarkers(1);
% end

headers=T(findMarkers(end):strfind(T,'H2O')+2);
headers=strsplit(headers);
T=strtrim(T);
remChars=['F�üè',char(160)];
rem=[];
for z=1:length(remChars)
rem=[rem,strfind(T,remChars(z))];
end; T(rem)=[];
tdfnd=min(strfind(T,'H2O'));
T=T(tdfnd+7:end);
sshfnd=min(strfind(T,'Session finished'));
if ~isempty(sshfnd);T=T(1:sshfnd-1);end;
T=strtrim(T);
T=T(find(isstrprop(T,'punct') | isstrprop(T,'digit') | isstrprop(T,'wspace')));
Tlines=splitlines(T);
textdata=[];
for line=1:length(Tlines)
    ln=Tlines{line};
    ln=ln(find(isstrprop(ln,'punct') | isstrprop(ln,'digit') | isstrprop(ln,'wspace')));
    ln=str2num(ln);
    if length(ln)<length(headers);
        ln(end+1:length(headers))=nan;end
    textdata=[textdata;ln(1:length(headers))];
end
for j=1:length(headers)
    behaviorVars.(headers{j})=textdata(:,j);
end

if iscell(session) && length(session)>1
    sessionsDone=1;
    behaviorVars.session=ones(size(behaviorVars.trial)).*sessionsDone;
    for x=2:length(session)
        sesh=cell2mat(session(x));
        T=fileread(sesh);
        findMarkers=strfind(T,'trial ');
        
        findMarkers(find(findMarkers>500))=[];
        if(isempty(findMarkers))
            findMarkers=strfind(T, 'n	');
            findMarkers = findMarkers(1);
        end

        headers=T(findMarkers(end):strfind(T,'H2O')+2);
        headers=strsplit(headers);
        T=strtrim(T);
        remChars=['F�üè',char(160)];
        rem=[];
        for z=1:length(remChars)
        rem=[rem,strfind(T,remChars(z))];
        end; T(rem)=[];
        tdfnd=min(strfind(T,'H2O'));
        T=T(tdfnd+7:end);
        sshfnd=min(strfind(T,'Session finished'));
        if ~isempty(sshfnd);T=T(1:sshfnd-1);end;
        T=strtrim(T);
        T=T(find(isstrprop(T,'punct') | isstrprop(T,'digit') | isstrprop(T,'wspace')));
        Tlines=splitlines(T);
        textdata=[];
        for line=1:length(Tlines)
            ln=Tlines{line};
            ln=ln(find(isstrprop(ln,'punct') | isstrprop(ln,'digit') | isstrprop(ln,'wspace')));
            ln=str2num(ln);
            textdata=[textdata;ln(1:length(headers))];
        end
        for j=1:length(headers)
            behaviorVars.(headers{j})=[behaviorVars.(headers{j}); textdata(:,j)];
        end
        sessionsDone=sessionsDone+1;
        behaviorVars.session=[behaviorVars.session; ones(size(textdata,1),1).*sessionsDone];
    end
end

end


