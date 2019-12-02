/** MSFS.C --> MASTER SOUND & FILE SYSTEM CODE **/
#include "msfs.h"

const Sint32		pcmbuftbl[19] = {
	PCMBUF6, PCMBUF7, PCMBUF8, PCMBUF9, PCMBUF10, PCMBUF11, PCMBUF12, PCMBUF13, PCMBUF14, PCMBUF15,
	PCMBUF16, PCMBUF17, PCMBUF18, PCMBUF19, PCMBUF20, PCMBUF21, PCMBUF22, PCMBUF23, PCMBUF24
};

p64pcm			pcm_slot[19];
snd_ring		music_buf[5];
int				musicPitch = S1536KHZ;
int				musicTimer = 64;
Sint8*			music = (Sint8*)"BP10.MUS";

int			CH_SND_NUM[8];
static Bool		ch_on[8];
static Bool		channel_ready[8];
static Bool		channel_busy[8];
static unsigned char channel_volpan[8];
int channel_playtimer[8];
static	int	mrd_pos = 0;
int			buf_pos;
int			music_frames = 0;
static	int	music_sectors = 0;
int			buffers_filled = 0;
int			fetch_timer = 0;
Bool			m_trig;
Bool			chg_music = false;

Uint32	rd_frames = 0;
Uint32	curRdFrame = 0;

//Play_reference is the sector offset for when we restart reading of the music file so it doesn't restart.
static int	play_ref = 0;
//File reference is the sector offset for when we restart reading of the data file.
static int	file_ref = 0;

//GFS Names
GfsHn	music_fs;
GfsHn	gfsx;
GfsHn	gfs_s;
GfsHn	gfs_h;

//Rate of data reading
//We will have to see if this is enough buffer time...
const int			m_step = (4 * 2048);
const int			m_sector = 4;
const int			mcpy_factor = 4;

const int			rt_step = (5 * 2048);
const int			rt_sector = 5;

const int			sf_step = (5 * 2048);
const int			sf_sector = 5;

//Read Control
static Bool	model_requested;
static Bool sound_requested;
static Bool map_requested;

request requests[19]; //ZTP Requests

request * activeZTP; //Pointers to currently serving file structures
p64pcm * activePCM;
_heightmap * activePGM;

unsigned char NactiveZTP = 0;
unsigned char NactivePCM = 0;
unsigned char NactivePGM = 0;

void * active_LWRAM_ptr = (void*)LWRAM;
void * active_HWRAM_ptr = (void*)LWRAM;
	
///Something more to do here, but it does work.
///This goes at interrupt. Control is performed by ch_on Boolean.
//Notice: slSoundRequest is broken into multiple parameters by newlines (comma).
void sound_on_channel(Uint8 sound_number, Uint8 channel){
bool	ready_play = false;
pcm_slot[sound_number].offset = 0;
		if(channel_ready[channel] == true){
	if(ch_on[channel] == true){
	if(channel_playtimer[channel] < 1){
		ready_play = true;
	}
	if(ready_play == true){
		slSoundRequest("bbwwwbb",
		SND_PCM_START,
		channel, //Channel [and other information, like bit depth and channel information, but its all zero]
		channel_volpan[channel], //Pan/Volume
		MAP_TO_SCSP(pcm_slot[sound_number].dstAddress + pcm_slot[sound_number].offset), //location [in sound RAM]
		(pcm_slot[sound_number].playsize),	//Size
		(pcm_slot[sound_number].pitchword), // Pitch
		0); //Effect and Effect Level [Mono]
		ready_play = false;
	}
	channel_playtimer[channel]++;
	if(channel_playtimer[channel] == 200 || channel_playtimer[channel] == 400 || channel_playtimer[channel] == 600 || channel_playtimer[channel] == 800 || channel_playtimer[channel] == 1000){
		ready_play = true;
	}
	if(pcm_slot[sound_number].pitchword == S1536KHZ){
	pcm_slot[sound_number].offset = channel_playtimer[channel] * S1536TIMER;
	if(channel_playtimer[channel] >= ( (pcm_slot[sound_number].playsize>>9) ) ){
		ch_on[channel] = false;
		}
	} else if(pcm_slot[sound_number].pitchword == S3072KHZ){
	pcm_slot[sound_number].offset = channel_playtimer[channel] * S3072TIMER;
	if(channel_playtimer[channel] >= ( (pcm_slot[sound_number].playsize>>10) ) ){
		ch_on[channel] = false;
		}
	}
}
//sound complete, close channel
	if(ch_on[channel] != true){
		slSoundRequest("b", SND_PCM_STOP, channel);
		pcm_slot[sound_number].offset = 0;
		ready_play = false;
		channel_playtimer[channel] = 0;
		channel_busy[channel] = false;
	}

	}
}
void	music_vblIn(Uint8 vol){
//NOTICE: Music PCM buffer currently sits at the last 160 KB of sound RAM. m_trig Boolean is the control variable of playback.
///Sample clipping issue? .. That's literally just the bitrate reduction.
	if(m_trig == true){
	if(fetch_timer == 0){
		slSoundRequest("bbwwwbb", SND_PCM_START, 0, vol<<5, (music_buf[buf_pos].play_pcmbuf), (32768), (musicPitch), 0);
	}
		fetch_timer++;
	if(fetch_timer >= musicTimer){
		//Injecting a PCM stop to hopefully save the ears of someone who uses a real console.
		slSoundRequest("b", SND_PCM_STOP, 0);
		buffers_filled -= 1;
		buf_pos++;
///Ring buffer wrap
	if(buf_pos > 4){
		buf_pos = 0;
	}
		fetch_timer = 0;
	}
	}
}

//The sound number is the order it was loaded in.
//The idea is a map file has a table of its sounds.
//Like sounds should always be in the same spot in the table.
//It is not neccessarily true though that these sounds are always the same size or in the same spot in sound RAM.
void	trigger_sound(Uint8 channel, Uint8 sound_number, Uint8 vp_word)
{
		if(pcm_slot[sound_number].file_done != true) return; //Avoid playing garbage on unloaded sounds
	if(channel_busy[channel] != true){
	channel_ready[channel] = true;
	CH_SND_NUM[channel] = sound_number;
	ch_on[channel] = true;
	channel_busy[channel] = true;
	channel_volpan[channel] = vp_word;
	}
}

void ztModelRequest(Sint8 * name, entity_t * model, char useHiMem, char sortType, short base_texture)
{
//Fill out the request.
	requests[NactiveZTP].filename = (Sint8*)GFS_NameToId(name);
	requests[NactiveZTP].tmodel = model;
	requests[NactiveZTP].active = true;
	requests[NactiveZTP].useHiMem = useHiMem;
	model->sortType = sortType;
	model->base_texture = base_texture;

	NactiveZTP++;
}

///Notice: In loading order, put sound requests below model requests.
void	p64SoundRequest(Sint8* name, Sint32 bitrate, Uint8 destBufSeg)
{
///Fill out the request.
	pcm_slot[NactivePCM].pitchword = bitrate;
	pcm_slot[NactivePCM].loctbl = destBufSeg;
	pcm_slot[NactivePCM].dstAddress = pcmbuftbl[destBufSeg];
	pcm_slot[NactivePCM].fid = (Sint8*)GFS_NameToId(name);
	pcm_slot[NactivePCM].active = true;

NactivePCM++;
}

void	p64MapRequest(Sint8* name, Uint8 mapNum)
{
///Fill out the request.
	maps[NactivePGM].dstAddress = hmap_buf;
	maps[NactivePGM].fid = (Sint8*)GFS_NameToId(name);
	maps[NactivePGM].active = true;

NactivePGM++;
}


void	file_request_loop(void)
{
	
	jo_printf(9, 0, "(%i)", NactiveZTP);
	jo_printf(12, 0, "(%i)", NactivePCM);
	jo_printf(15, 0, "(%i)", NactivePGM);
		//PRINT STATEMENTS IN LOOPS = BAD
	for(unsigned char i = 0; i < 19; i++){
		if(requests[i].active == true){
			activeZTP = &requests[i]; //Remember: "&" qualifies "address of". You kind of thought that as what a struct would be in the first place?
			model_requested = true; //Too bad.
			break;
		} else if(pcm_slot[i].active == true){
			activePCM = &pcm_slot[i]; 
			sound_requested = true;
			break;
		} else if(i < 4 && maps[i].active == true){
			activePGM = &maps[i];
			map_requested = true;
			break;
		} else {
			continue;
		}
	}
	
}

void	pop_load_ztp(void(*game_code)(void)){
	Sint32			rdsize;
	Sint32			nsct_f;
	Sint32			fsize;
	Sint32			stat;
    void * workAddress = (void*)LWRAM;
//Model Data is the destination of data in the work area.
	modelData_t bufModelX;
//Destination address?
	void * ptr2 = &bufModelX;
									/** PROCESS REQUEST CODE **/
//Open GFS
	gfsx = GFS_Open((Sint32)activeZTP->filename);
//Get sectors
	GFS_GetFileSize(gfsx, NULL, &nsct_f, NULL);
	GFS_GetFileInfo(gfsx, NULL, NULL, &fsize, NULL);

//How many frames are we reading?
	if(rt_step < fsize){
		rd_frames = (fsize + (rt_step - 1))/(rt_step);
	} else {
		rd_frames = 1;
	}
//Seek to the desired spot on the file
	GFS_Seek(gfsx, file_ref, GFS_SEEK_SET);
//Set up & Read from CD to CD buffer
	GFS_SetReadPara(gfsx, rt_step);
	GFS_SetTransPara(gfsx, rt_sector);
//Transfer mode should be CPU since the destination is LWRAM, which the SCU cannot access
///SDMA0 seems ok in this case, but watch out for errors.
	GFS_SetTmode(gfsx, GFS_TMODE_CPU);
//TIP: GFS_Nw_CdRead cannot be checked for completion. Sorry!
	GFS_NwCdRead(gfsx, fsize);
									/** END PROCESS REQUEST CODE **/
for( ; fetch_timer >= (mcpy_factor * 1) && fetch_timer <= musicTimer && mrd_pos == buf_pos ; ){
	workAddress = (activeZTP->useHiMem == true) ? active_HWRAM_ptr : active_LWRAM_ptr;
	GFS_NwFread(gfsx, rt_sector, (Uint32*)(workAddress +(curRdFrame * rt_step)), rt_step);
	curRdFrame++;
//File seek reference += number of sectors.
	file_ref += rt_sector;
do{
		game_code();
		GFS_NwExecOne(gfsx);
		GFS_NwGetStat(gfsx, &stat, &rdsize);
	// jo_printf(0, 16, "(%i)", fsize);
	// jo_printf(0, 20, "(10) loop label");
	// jo_printf(0, 15, "(%i)", curRdFrame);
	// jo_printf(4, 15, "(%i)", rd_frames);
	// jo_printf(0, 22, "(%i) cur rd case", mrd_pos);
	// jo_printf(0, 23, "(%i) cur buf case", buf_pos);
		// jo_printf(0, 7, "(%i)", music_frames);
		// jo_printf(7, 7, "(%i) sct off", play_ref);
		// jo_printf(0, 10, "(%i) fs stat", stat);
		// jo_printf(0, 11, "(%i) fetch", fetch_timer);
}while(stat != GFS_SVR_COMPLETED && rdsize < rt_step);
	//FOR-DO-WHILE READ LOOP END STUB
	}
	GFS_Close(gfsx);
										/** FILE REQUEST HANDLER FOR ZTP **/
	if(curRdFrame >= rd_frames){
		if(activeZTP->file_done != true){
 	// Copy gfsx payload to modelData_t bufmodel
	slDMACopy(workAddress, ptr2, sizeof(modelData_t));
	//ADDED
    activeZTP->tmodel->nbMeshes = bufModelX.TOTAL_MESH;
	activeZTP->tmodel->nbFrames = bufModelX.nbFrames;
	
	//Uint16 first_texture = loadTextures(workAddress, &bufModelX);
	Sint32 bytesOff = bufModelX.TEXT_SIZE+(sizeof(modelData_t));
	workAddress = (workAddress + bytesOff);
	
	activeZTP->tmodel->size = (unsigned int)workAddress;
	workAddress = loadPDATA((workAddress), activeZTP->tmodel, &bufModelX);
	activeZTP->tmodel->size = (unsigned int)workAddress - activeZTP->tmodel->size;


	setTextures(activeZTP->tmodel, activeZTP->tmodel->base_texture, &activeZTP->tmodel->numTexture);
    workAddress = loadAnimations(workAddress, activeZTP->tmodel, &bufModelX);
	
		//Decimate existing sort type bits
	activeZTP->tmodel->pol[0]->attbl[0].sort &= 252;
		//Inject new sort type bits
	activeZTP->tmodel->pol[0]->attbl[0].sort |= activeZTP->tmodel->sortType;
		//New render path only reads first attbl for sorting

	if(activeZTP->useHiMem == true) //High / low work area switch
	{
	active_HWRAM_ptr = workAddress;
	} else {
	active_LWRAM_ptr = workAddress;
	}
	
	// jo_printf(0, 12, "MODEL");
	// jo_printf(0, 13, "(%i)", bufModelX.TOTAL_MESH);
	// jo_printf(3, 13, "(%i)", bufModelX.PDATA_SIZE);
	// jo_printf(10, 13, "(%i)", activeZTP->tmodel->nbMeshes);
		NactiveZTP--;
		file_ref = 0;
		rd_frames = 0;
		curRdFrame = 0;
	activeZTP->tmodel->file_done = true;
	activeZTP->active = false;
	model_requested = false;
		}
	//FILE HANDLER END STUB
	}
//PROCESS REQUEST END STUB
}

void	pop_load_pcm(void(*game_code)(void)){
	Sint32	fsizeS;
	Sint32	nsctS;
	Sint32	rdsize;
	Sint32	stat;

	gfs_s = GFS_Open((Sint32)activePCM->fid);
	
	GFS_GetFileSize(gfs_s, NULL, &nsctS, NULL);
	GFS_GetFileInfo(gfs_s, NULL, NULL, &fsizeS, NULL);
///How many frames are we reading?
	if(sf_step < fsizeS){
		rd_frames = (fsizeS + (sf_step - 1))/(sf_step);
	} else {
		rd_frames = 1;
	}

///Seek to the desired spot on the file
	GFS_Seek(gfs_s, file_ref, GFS_SEEK_SET);
	GFS_SetReadPara(gfs_s, sf_step);
	GFS_SetTransPara(gfs_s, sf_sector);
///Transfer mode should be SCU because this is going to sound RAM.
	GFS_SetTmode(gfs_s, GFS_TMODE_SCU);
	GFS_NwCdRead(gfs_s, fsizeS);
	
	for( ; fetch_timer >= (mcpy_factor * 1) && fetch_timer <= musicTimer && mrd_pos == buf_pos ; ){
		GFS_NwFread(gfs_s, sf_sector, (Sint32*)(activePCM->dstAddress + (curRdFrame * sf_step)), sf_step);
		file_ref += sf_sector;
		do{
			game_code();
			GFS_NwExecOne(gfs_s);
			GFS_NwGetStat(gfs_s, &stat, &rdsize);
	// jo_printf(0, 20, "(17) loop label");
	// jo_printf(0, 16, "(%i)", fsizeS);
	// jo_printf(0, 15, "(%i)", curRdFrame);
	// jo_printf(4, 15, "(%i)", rd_frames);
	// jo_printf(0, 22, "(%i) cur rd case", mrd_pos);
	// jo_printf(0, 23, "(%i) cur buf case", buf_pos);
		// jo_printf(0, 7, "(%i)", music_frames);
		// jo_printf(7, 7, "(%i) sct off", play_ref);
		// jo_printf(0, 10, "(%i) fs stat", stat);
		// jo_printf(0, 11, "(%i) fetch", fetch_timer);
		}while(stat != GFS_SVR_COMPLETED && rdsize < sf_step);

		curRdFrame++;
	///FOR-DO-WHILE READ LOOP END STUB
	}
	GFS_Close(gfs_s);
		if(curRdFrame >= rd_frames){
			if(activePCM->file_done != true){
///How many buffers is the sound going to consume?
			if(fsizeS > 16384){
				activePCM->segments	= (fsizeS + (16384 - 1))/(16384);
			} else {
				activePCM->segments 	= 1;
			}
			activePCM->playsize = fsizeS;
			activePCM->frames = rd_frames;
			
			NactivePCM--;
			file_ref = 0;
			rd_frames = 0;
			curRdFrame = 0;
		activePCM->file_done = true;
		activePCM->active = false;
		sound_requested = false;
			}
		///SFX HANDLER END STUB
		}
///SOUND LOAD REQUEST END STUB
}

void	pop_load_map(void(*game_code)(void)){
	Sint32	fsizeH;
	Sint32	nsctH;
	Sint32	rdsize;
	Sint32	stat;

	gfs_h = GFS_Open((Sint32)activePGM->fid);
	
	GFS_GetFileSize(gfs_h, NULL, &nsctH, NULL);
	GFS_GetFileInfo(gfs_h, NULL, NULL, &fsizeH, NULL);
///How many frames are we reading?
	if(sf_step < fsizeH){
		rd_frames = (fsizeH + (sf_step - 1))/(sf_step);
	} else {
		rd_frames = 1;
	}

///Seek to the desired spot on the file
	GFS_Seek(gfs_h, file_ref, GFS_SEEK_SET);
	GFS_SetReadPara(gfs_h, sf_step);
	GFS_SetTransPara(gfs_h, sf_sector);
///Transfer mode should be SCU because this is going to sound RAM.
	GFS_SetTmode(gfs_h, GFS_TMODE_SCU);
	GFS_NwCdRead(gfs_h, fsizeH);
	
	for( ; fetch_timer >= (mcpy_factor * 1) && fetch_timer <= musicTimer && mrd_pos == buf_pos ; ){
		GFS_NwFread(gfs_h, sf_sector, (Sint32*)(activePGM->dstAddress + (curRdFrame * sf_step)), sf_step);
		file_ref += sf_sector;
		do{
			game_code();
			GFS_NwExecOne(gfs_h);
			GFS_NwGetStat(gfs_h, &stat, &rdsize);
	// jo_printf(0, 20, "(88) loop label");
	// jo_printf(0, 16, "(%i)", fsizeH);
	// jo_printf(0, 15, "(%i)", curRdFrame);
	// jo_printf(4, 15, "(%i)", rd_frames);
	// jo_printf(0, 22, "(%i) cur rd case", mrd_pos);
	// jo_printf(0, 23, "(%i) cur buf case", buf_pos);
		// jo_printf(0, 7, "(%i)", music_frames);
		// jo_printf(7, 7, "(%i) sct off", play_ref);
		// jo_printf(0, 10, "(%i) fs stat", stat);
		// jo_printf(0, 11, "(%i) fetch", fetch_timer);
		}while(stat != GFS_SVR_COMPLETED && rdsize < sf_step);

		curRdFrame++;
	///FOR-DO-WHILE READ LOOP END STUB
	}
	GFS_Close(gfs_h);
		if(curRdFrame >= rd_frames){
			if(activePGM->file_done != true){
				read_gmp_header(activePGM);
				//
					if(JO_IS_ODD(activePGM->Xval) && JO_IS_ODD(activePGM->Zval)){
				for(int i = 0; i < (activePGM->totalPix); i++){
				volatile Uint8	* tempVert = (volatile Uint8 *)activePGM->dstAddress + i;
				buf_map[i] = *tempVert;
				}
				// jo_printf(8, 20, "(%i)", activePGM->totalPix);
				// jo_printf(15, 20, "(%i)", activePGM->Xval);
				// jo_printf(20, 20, "(%i)", activePGM->Zval);
					} else {
				jo_printf(8, 25, "MAP REJECTED - IS EVEN");
					}

			NactivePGM--;
			file_ref = 0;
			rd_frames = 0;
			curRdFrame = 0;
		activePGM->file_done = true;
		activePGM->active = false;
		map_requested = false;
			}
		///HMAP HANDLER END STUB
		}
///MAP LOAD REQUEST END STUB
}

void		master_file_system(void(*game_code)(void))
{	
	
music_buf[0].rd_pcmbuf = (void*)PCMBUF4;
music_buf[0].play_pcmbuf = MAP_TO_SCSP(PCMBUF1);
music_buf[1].rd_pcmbuf = (void*)PCMBUF5;
music_buf[1].play_pcmbuf = MAP_TO_SCSP(PCMBUF2);
music_buf[2].rd_pcmbuf = (void*)PCMBUF1;
music_buf[2].play_pcmbuf = MAP_TO_SCSP(PCMBUF3);
music_buf[3].rd_pcmbuf = (void*)PCMBUF2;
music_buf[3].play_pcmbuf = MAP_TO_SCSP(PCMBUF4);
music_buf[4].rd_pcmbuf = (void*)PCMBUF3;
music_buf[4].play_pcmbuf = MAP_TO_SCSP(PCMBUF5);
	if(chg_music == true){
		m_trig = false;
		slSoundRequest("b", SND_PCM_STOP, 0);
			buffers_filled = 0;
			fetch_timer = 0;
			mrd_pos = 0;
			buf_pos = 0;
			music_frames = 0;
			play_ref = 0;
		chg_music = false;
	}
	if(musicPitch == S3072KHZ){
		musicTimer = 32;
	} else if(musicPitch == S1536KHZ){
		musicTimer = 64;
	}
					/**MUSIC SYSTEM SETUP**/
	//This function is supposed to play a mono PCM sound effect constantly. BY THE WAY: This system is currently unprepared for changing music files.
static Sint32			fid_m;
static Sint32			msize;
static Sint32			nsct_m;
static Sint32			stat;
static Sint32			rdsize;
					fid_m = GFS_NameToId(music);
					music_fs = GFS_Open(fid_m);
//Get sectors
//HEY! SBL DOCUMENTATION IS WRONG! THIRD ITEM nzect IS GFS SECTOR COUNT. SECOND ITEM IS CD SECTOR SIZE.
//TIP: MEMORY MUST BE MANAGED IN SECTORS (2KB)
	GFS_GetFileSize(music_fs, NULL, &nsct_m, NULL);
	GFS_GetFileInfo(music_fs, NULL, NULL, &msize, NULL);
//How many frames are we reading?
	if(m_step < msize){
		music_sectors = (msize + (m_step - 1))/(m_step);
	}
	
	GFS_SetReadPara(music_fs, m_step);
	GFS_SetTransPara(music_fs, m_sector);
//transfer should be SCU type since it's going from B to A bus.
	GFS_SetTmode(music_fs, GFS_TMODE_SDMA1);
//Seek music to the desired location
	GFS_Seek(music_fs, play_ref, GFS_SEEK_SET);
//MUSIC loading follows
	GFS_NwCdRead(music_fs, (32 * 2048));

//Let's make a pre-playback loop to read from the CD to fill a series of buffers using a work buffer.
for( ; buffers_filled < 3  && m_trig != true ; ){
 	if(music_frames < mcpy_factor && buffers_filled < 3){
	GFS_NwFread(music_fs, m_sector, (void*)music_buf[buffers_filled].rd_pcmbuf + (music_frames * m_step), m_step);
	play_ref += m_sector;
	music_frames++;
	}
do{
		game_code();
		GFS_NwExecOne(music_fs);
		GFS_NwGetStat(music_fs, &stat, &rdsize);
	if(music_frames == mcpy_factor){
	buffers_filled += 1;
	music_frames = 0;
	}
	// jo_printf(0, 20, "(49) loop label");
	// jo_printf(0, 22, "(%i) buffers filled", buffers_filled);
	// jo_printf(0, 23, "(%i) cur buf case", buf_pos);
		// jo_printf(0, 7, "(%i)", music_frames);
		// jo_printf(7, 7, "(%i) sct off", play_ref);
		// jo_printf(0, 10, "(%i) fs stat", stat);
		// jo_printf(0, 11, "(%i) fetch", fetch_timer);
	mrd_pos = 3;
	buf_pos = 3;
}while(stat != GFS_SVR_COMPLETED && rdsize < m_step);
}
/**
MAIN MUSIC-GAME LOOP
Fault Tolerance needs study.
Sample clipping needs correction.
Data M_TRIG : Music trigger. Sets Vblank buffer playback toggle ON.
Data music_frames : Frames read from music file system under copy factor.
Data mcpy_factor : Frames to read from music file to complete an 16-sector buffer.
Data mrd_pos : Last or current read case in music buffer. It is the last position in the case of file system interrupting music.
Data buf_pos : The current playback case in music buffer. Synchronized with mrd_pos at file system interrupt (NOTE: not system IRQ, see break)
Data music_fs : GFS handle of music file. [Gets closed and re-opened. Music and file system can operate with max open files of 2]
Data m_sector : Number of sectors to fetch from CD block buffer.
Data music_buf : Music ring buffer struct array. Ring buffer is comprised of numerous 16-sector buffers. [At 30.72KHz mono, that is 8 frames of sound]
Data m_step : Bytes to fetch from CD block buffer. Also used as address offset (in the case of sub-8 sectors used as m_sector ; > 1 used as mcpy_factor)
Data play_ref : Seek reference when music file is re-opened.
Data rdsize : Fetch size of single read loop increment.
Data stat : Execution error/status information. If it is ever 2, you have a problem.
Function game_code : Your game.
buffers_filled : Unnecessary, but notifies system how many buffers have new data. It is incremented down at Vblank IRQ.

FILE SYSTEM INTERRUPT
Bool model_requested : Trigger in game code as to whether or not file process is requested.
Data fetch_timer : Buffer switch timer. See Vblank IRQ. This timer will increment buf_pos which triggers reads until mrd_pos equals buf_pos.
**/
for( ; ; ){
	if(play_ref < nsct_m){
	m_trig = true;
	} else if(play_ref >= nsct_m){
	m_trig = false;
	slSoundRequest("b", SND_PCM_STOP, 0);
		buffers_filled = 0;
		fetch_timer = 0;
		mrd_pos = 0;
		buf_pos = 0;
		music_frames = 0;
		play_ref = 0;
	break;
	}
 	if(music_frames < mcpy_factor && mrd_pos != buf_pos){
	GFS_NwFread(music_fs, m_sector, (void*)music_buf[mrd_pos].rd_pcmbuf + (music_frames * m_step), m_step);
	play_ref += m_sector;
	music_frames++;
	}
do{
		game_code();
		GFS_NwExecOne(music_fs);
		GFS_NwGetStat(music_fs, &stat, &rdsize);
	if(music_frames == mcpy_factor){
		if(mrd_pos != buf_pos){mrd_pos++;}
		buffers_filled += 1;
		music_frames = 0;}
	// jo_printf(0, 15, "(%i)", curRdFrame);
	// jo_printf(4, 15, "(%i)", rd_frames);
	// jo_printf(0, 18, "(%i)", msize);
	// jo_printf(0, 20, "(49) loop label");
	// jo_printf(0, 22, "(%i) cur rd case", mrd_pos);
	// jo_printf(0, 23, "(%i) cur play case", buf_pos);
		// jo_printf(0, 7, "(%i)", music_frames);
		// jo_printf(7, 7, "(%i) sct off", play_ref);
		// jo_printf(0, 10, "(%i) fs stat", stat);
		// jo_printf(0, 11, "(%i) fetch", fetch_timer);
}while(stat != GFS_SVR_COMPLETED && rdsize < m_step);
if(mrd_pos > 4){mrd_pos = 0;}
if((model_requested == true || sound_requested == true || map_requested == true || chg_music == true) && fetch_timer >= (mcpy_factor * 1) && fetch_timer <= musicTimer){if(mrd_pos == buf_pos){break;}}
}
					/**END MUSIC SYSTEM SETUP**/
	GFS_Close(music_fs);
										/**MASTER FILE SYSTEM LOOP**/
if(model_requested == true){
	pop_load_ztp(game_code);
	} else if(sound_requested == true){
		pop_load_pcm(game_code);
		} else if(map_requested == true){
			pop_load_map(game_code);
			}
										/**END FILE SYSTEM LOOP**/
}

