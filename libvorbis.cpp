// Music Room BGM Library
// ----------------------
// libvorbis.cpp - Ogg Vorbis stuff
// ----------------------
// "©" Nmlgc, 2011

#include "platform.h"
#include "infostruct.h"
#include "packmethod.h"
#include "ui.h"
#include "libvorbis.h"
#include <FXFile.h>
#include <vorbis/vorbisenc.h>

const int OV_BLOCK = 8192;

// Vorbis Stuff
// ------------
size_t FXFile_read(void* _DstBuf, size_t _Dummy_, size_t _Count, FXFile* _File)
{
	return _File->readBlock(_DstBuf, _Count);
}

int FXFile_seek(FXFile* _File, ogg_int64_t off, int whence)
{
	return _File->position(off, whence);
}

int FXFile_close(FXFile* _File)
{
	return _File->close();
}

int FXFile_tell(FXFile* _File)
{
	return _File->position();
}
// ------------

bool DumpDecrypt(GameInfo* GI, TrackInfo* TI, const FXString& OutFN)
{
	bool Ret = false;
	FXFile Src;

	if(!GI->OpenBGMFile(Src, TI))	return Ret;
	Ret = GI->PM->Dump(GI, Src, TI->GetStart(), TI->FS, OutFN);
	Src.close();
	return Ret;
}

// Virtual file
// ------------
VFile::VFile()
{
	Buf = NULL;
	Size = Read = Write = 0;
}

VFile::VFile(const ulong& _Size)
{
	Create(_Size);
}

void VFile::Create(const ulong& _Size)
{
	Size = _Size;
	Read = Write = 0;
	Buf = (char*)malloc(Size);
}

void VFile::Clear()
{
	Size = Read = Write = 0;
	free(Buf);
	Buf = NULL;
}

VFile::~VFile()
{
	Clear();
}

size_t VFile_read(void* _DstBuf, size_t _Dummy_, size_t _Count, VFile* VF)
{
	if(!VF->Buf)	return 0;

	size_t Rem = VF->Write - VF->Read;
	if(Rem < _Count) _Count = Rem;

	memcpy(_DstBuf, VF->Buf + VF->Read, _Count);
	VF->Read += _Count;
	return _Count;
}

int VFile_seek(VFile* VF, ogg_int64_t off, int whence)
{
	switch(whence)
	{
	case SEEK_SET:	VF->Read = off;
					return 0;
	case SEEK_CUR:	VF->Read += off;
					return 0;
	case SEEK_END:	VF->Read = VF->Size - off;
					return 0;
	default:		return -1;
	}
}

int VFile_tell(VFile* VF)
{
	return VF->Read;
}
// ------------

// Encoding state
// --------------
OggVorbis_EncState::OggVorbis_EncState()
{
	out = NULL;
	memset(&stream_out, 0, sizeof(ogg_stream_state));
	vorbis_info_init(&vi);
	memset(&vd, 0, sizeof(vorbis_dsp_state));
	memset(&vb, 0, sizeof(vorbis_block));
}

bool OggVorbis_EncState::setup(FXFile* _out, const float& freq, const float& quality)
{
	int ret;

	out = _out;
	vorbis_info_init(&vi);
	ret = vorbis_encode_init_vbr(&vi, 2, freq, quality);
	if(ret)	return false;
	ret = vorbis_analysis_init(&vd,&vi);
	ret = vorbis_block_init(&vd,&vb);
	return true;
}

bool OggVorbis_EncState::write_headers()
{
	vorbis_comment vc;
	bool ret;

	vorbis_comment_init(&vc);
	ret = write_headers(&vc);
	vorbis_comment_clear(&vc);
	return ret;
}

bool OggVorbis_EncState::write_headers(vorbis_comment* vc)
{
	ogg_page og;
	ogg_packet header;
    ogg_packet header_comm;
    ogg_packet header_code;

	if(!vc && !stream_out.body_data)	return false;
	
	vorbis_analysis_headerout(&vd, vc, &header,&header_comm,&header_code);
    ogg_stream_packetin(&stream_out,&header); // automatically placed in its own page
	ogg_stream_packetin(&stream_out,&header_comm);
	ogg_stream_packetin(&stream_out,&header_code);
    
	// This ensures the actual audio data will start on a new page, as per spec
	while(ogg_stream_flush(&stream_out,&og))
	{
		out->writeBlock(og.header,og.header_len);
		out->writeBlock(og.body,og.body_len);
	}
	return true;
}

bool OggVorbis_EncState::encode_pcm(char* buf, const int& size)
{
	ogg_packet op;
	float **buffer;
	long i;

	// expose the buffer to submit data
	buffer=vorbis_analysis_buffer(&vd,size/4);

	// uninterleave samples
	for(i=0;i<size/4;i++)
	{
		buffer[0][i]=((buf[i*4+1]<<8)| (0x00ff&(int)buf[i*4]))/32768.f;
		buffer[1][i]=((buf[i*4+3]<<8)| (0x00ff&(int)buf[i*4+2]))/32768.f;
	}

	// tell the library how much we actually submitted
	vorbis_analysis_wrote(&vd,i);

	if(!stream_out.body_data)	return false;

	/* vorbis does some data preanalysis, then divvies up blocks for
	   more involved (potentially parallel) processing.  Get a single
	   block for encoding now */
	while(vorbis_analysis_blockout(&vd,&vb)==1)
	{
		// analysis, assume we want to use bitrate management
		vorbis_analysis(&vb,NULL);
		vorbis_bitrate_addblock(&vb);

		while(vorbis_bitrate_flushpacket(&vd,&op))
		{
			ogg_write_packet(*out, &stream_out, &op);
		}
	}
	return true;
}

bool OggVorbis_EncState::encode_file(FXFile& in, const ulong& bytes, char* buf, const ulong& bufsize, volatile FXulong& d, volatile bool* StopReq)
{
	ulong Rem = bytes;

	while(Rem > 0 && !(*StopReq))
	{
		int Read = MIN(bufsize, Rem);
		in.readBlock(buf, Read);
		Rem -= Read;
		encode_pcm(buf, Read);
		d += Read;
	}
	// Finalize
	if(!(*StopReq))	encode_pcm(NULL, 0);
	return !(*StopReq);
}

void OggVorbis_EncState::clear()
{
	vorbis_block_clear(&vb);
	vorbis_dsp_clear(&vd);
	vorbis_info_clear(&vi);
}

OggVorbis_EncState::~OggVorbis_EncState()
{
	clear();
}
// --------------

// Decodes [Size] bytes from [vf] into [buffer]. Loops according to the info in [TI].
ogg_int64_t ov_read_bgm(OggVorbis_File* vf, char* buffer, const ulong& size, TrackInfo* TI)
{
	int Link;
	long Ret = 1;
	long Rem = size;
	ogg_int64_t Cur = ov_pcm_tell(vf);

	ulong L, E;
	TI->GetPos(FMT_SAMPLE, false, NULL, &L, &E);

	while(Rem > 0)
	{
		Ret = ov_read(vf, buffer + size - Rem, Rem, 0, 2, 1, &Link);
		Cur = ov_pcm_tell(vf);
		if(Cur >= E)
		{
			ov_pcm_seek_lap(vf, L != E ? L : 0);
			Ret -= (Cur - E) * 4;
			Cur = ov_pcm_tell(vf);
		}
		Rem -= Ret;
	}
	return Cur;
}

// Ogg packet copy functions
// ===============

int ov_bitstream_seek(OggVorbis_File* vf, ogg_int64_t pos, bool seek_to_header)
{
	ogg_int64_t seek;
	int link;

	ogg_int64_t total=ov_pcm_total(vf,-1);

	// which bitstream section does this pcm offset occur in?
	for(link=vf->links-1;link>=0;link--)
	{
		total-=vf->pcmlengths[link*2+1];
		if(pos>=total)break;
	}

	if(seek_to_header)	seek = vf->offsets[link];
	else				seek = vf->dataoffsets[link];

	ov_raw_seek(vf, seek);
	
	return link;
}

void vorbis_write_headers(FXFile& out, ogg_stream_state* os, vorbis_info* vi, vorbis_comment* vc)
{
	ogg_page og;
	ogg_packet header;
    ogg_packet header_comm;
    ogg_packet header_code;
	vorbis_dsp_state vd;

	vorbis_analysis_init(&vd, vi);
    vorbis_analysis_headerout(&vd, vc, &header,&header_comm,&header_code);
    ogg_stream_packetin(os,&header); // automatically placed in its own page
	ogg_stream_packetin(os,&header_comm);
	ogg_stream_packetin(os,&header_code);
    
	// This ensures the actual audio data will start on a new page, as per spec
	while(ogg_stream_flush(os,&og))
	{
		out.writeBlock(og.header,og.header_len);
		out.writeBlock(og.body,og.body_len);
	}
	
	vorbis_dsp_clear(&vd);
}

void vorbis_write_headers(FXFile& out, ogg_stream_state* os, vorbis_info* vi, ogg_packet* header, vorbis_comment* vc, ogg_packet* header_code)
{
	ogg_page og;
	ogg_packet dump[2];
    ogg_packet header_comm;
	vorbis_dsp_state vd;

	// Because of lolvorbis "we declare everything _we_ see no public use for static",
	// we have to needlessly include [vi] as parameter, and allocate two fake packets
	// just to make it create that one comment packet. Great.
	vorbis_analysis_init(&vd, vi);
    vorbis_analysis_headerout(&vd, vc, &dump[0],&header_comm,&dump[1]);
    ogg_stream_packetin(os,header); // automatically placed in its own page
	ogg_stream_packetin(os,&header_comm);
	ogg_stream_packetin(os,header_code);
    
	// This ensures the actual audio data will start on a new page, as per spec
	while(ogg_stream_flush(os,&og))
	{
		out.writeBlock(og.header,og.header_len);
		out.writeBlock(og.body,og.body_len);
	}
	
	vorbis_dsp_clear(&vd);
}

// Adapted from vcut.c
// -------
typedef struct {
	int length;
	unsigned char *packet;
} vcut_packet;

// Copies [packet] to [p]
static bool save_packet(vcut_packet* p, ogg_packet *packet)
{
	p->length = packet->bytes;
	p->packet = (unsigned char*)realloc(p->packet, p->length);
	if(!p->packet)
		return false;

	memcpy(p->packet, packet->packet, p->length);
	return true;
}

// Returns... packet sample count... or something like that
static long get_blocksize(vorbis_info* vi, ogg_packet *op, int& prevW)
{
	int size = vorbis_packet_blocksize(vi, op);
	int ret = (size+prevW)/4;
 
	prevW = size;
	return ret;
}

// Reads an arbitrary amount of bytes from [file_in] into [sync_in].
// Return value: number of read bytes
int ogg_update_sync(FXFile& file_in, ogg_sync_state* sync_in)
{
	static const ulong Read = 4096;

	char *buffer = ogg_sync_buffer(sync_in, Read);
	int bytes = file_in.readBlock(buffer, Read);
	ogg_sync_wrote(sync_in, bytes);
	return bytes;
}

// Writes pages to the given file, or discards them if file is NULL.
bool write_pages_to_file(ogg_stream_state *stream, FXFile& file, int flush)
{
	ogg_page page;

	if(flush)
	{
		while(ogg_stream_flush(stream, &page))
		{
			if(!file.isOpen()) continue;
			if(file.writeBlock(page.header,page.header_len) != page.header_len)
				return false;
			if(file.writeBlock(page.body,page.body_len) != page.body_len)
				return false;
		}
	}
	else
	{
		while(ogg_stream_pageout(stream, &page))
		{
			if(!file.isOpen()) continue;
			if(file.writeBlock(page.header,page.header_len) != page.header_len)
				return false;
			if(file.writeBlock(page.body,page.body_len) != page.body_len)
				return false;
		}
	}

	return true;
}

// Submits [packet] to [stream_out], and writes filled pages to [out].
bool ogg_write_packet(FXFile& out, ogg_stream_state* stream_out, ogg_packet *packet)
{
	int flush;

	/* According to the Vorbis I spec, we need to flush the stream after:
	 *  - the first (BOS) header packet
	 *  - the last header packet (packet #2)
	 *  - the second audio packet (packet #4), if the stream starts at
	 *    a non-zero granulepos */
	flush = (stream_out->packetno == 2)
			|| (stream_out->packetno == 4 && packet->granulepos != -1)
			|| packet->b_o_s || packet->e_o_s;
	ogg_stream_packetin(stream_out, packet);

	if(!write_pages_to_file(stream_out, out, flush))
	{
		BGMLib::UI_Stat_Safe("\nCouldn't write packet to output file\n");
		return false;
	}

	return true;
}

// Copies audio packets from [file_in] to [file_out].
// Stops once a given number of samples, or the end of the input stream is reached
ogg_int64_t ogg_packetcopy(FXFile& file_out, ogg_stream_state* stream_out, FXFile& file_in, ogg_stream_state* stream_in, ogg_sync_state* sync_in, vorbis_info* info_in, ogg_int64_t sample_end, ogg_int64_t sample_start)
{
	bool eos = false;
	bool write = (sample_start == 0);
	ogg_page og;
	ogg_packet op;
	ogg_int64_t granulepos = 0;
	vcut_packet last_packet;	// Last packet before [sample_start]
	// I don't know anything about what this variable is supposed to be, but it needs to be in this scope and it needs to be initialized with zero
	int prevW = 0;

	if(sample_end == 0)	return 0;

	if(sample_end < 0)	sample_end = 0x7fffffffffffffff + sample_end;

	memset(&last_packet, 0, sizeof(vcut_packet));
	
	while(!eos)
	{
		int result=ogg_sync_pageout(sync_in,&og);
		if(result<0)
		{ // missing or corrupt data at this page position
			BGMLib::UI_Stat_Safe("\nCorrupt or missing data in bitstream; continuing...\n");
		}
		else if(result != 0)
		{
			// Init [stream_in] if necessary
			if(stream_in->body_data == NULL)
			{
				ogg_stream_init(stream_in, ogg_page_serialno(&og));
			}
			result = ogg_stream_pagein(stream_in,&og);
			// If this fails, we have a broken Ogg, so we return to limit the damage
			if(result<0)	eos = true;
			while(!eos)
			{
				result=ogg_stream_packetout(stream_in,&op);
				if(result<=0)break; // need more data
				// Don't copy headers, we're always submitting them ourselves
				if(op.packetno < 3)	continue;

				if(info_in)
				{
					long new_gp = get_blocksize(info_in, &op, prevW);
					if(new_gp > -1 && stream_in->packetno > 4)	granulepos += new_gp;

					if(!write)
					{
						if(granulepos >= sample_start)
						{
							// Write out last packet
							ogg_packet lp;

							lp.b_o_s = 0;
							lp.e_o_s = 0;
							lp.granulepos = 0;
							lp.packetno = stream_out->packetno;
							lp.bytes = last_packet.length;
							lp.packet = last_packet.packet;

							ogg_write_packet(file_out, stream_out, &lp);
							write = true;
						}
						else
						{
							// Might be the last packet before [sample_start]. Save it
							save_packet(&last_packet, &op);
						}
					}
					if(sample_start != 0 && write)
					{
						// If the packet supplies a granulepos value, don't overwrite it!
						if(op.granulepos == -1)	op.granulepos = granulepos;
						op.granulepos -= sample_start;
					}
					if((granulepos - sample_start) > sample_end)
					{
						op.granulepos = sample_end;
						op.e_o_s = 1;
					}
				}

				if(op.e_o_s)	eos = write = true;
				if(write)	ogg_write_packet(file_out, stream_out, &op);
			}
		}
		else if(!eos)	eos = ogg_update_sync(file_in, sync_in) == 0;
	}
	SAFE_FREE(last_packet.packet);

	return granulepos;
}

ogg_int64_t ogg_packetcopy(FXFile& file_out, ogg_stream_state* stream_out, OggVorbis_File* ov_in, ogg_int64_t sample_end, ogg_int64_t sample_start)
{
	return ogg_packetcopy(file_out, stream_out, *((FXFile*)ov_in->datasource), &ov_in->os, &ov_in->oy, ov_in->vi, sample_end, sample_start);
}
// -------

// Quality to bitrate mapping. Adapted from vorbisenc.c.
// C++ lesson #?: Too much static enforcement can piss off coders, because it forces them to copy stuff!
// -------
const double rate_mapping_44_stereo[12]=
{
  22500.,32000.,40000.,48000.,56000.,64000.,
  80000.,96000.,112000.,128000.,160000.,250001.
};

float vorbis_quality_to_bitrate(const float& q)
{
	float ds = 0.0, _is;
	int is = 0.0;

	ds =modf(q, &_is);

	if(ds < 0)	{is = _is;	ds = 1.0+ds;}
	else		{is = _is+1;}

	return((rate_mapping_44_stereo[is]*(1.-ds)+rate_mapping_44_stereo[is+1]*ds)*2.);
}
// -------

// ===============
