// Music Room BGM Library
// ----------------------
// libvorbis.h - Ogg Vorbis stuff
// ----------------------
// "©" Nmlgc, 2011

#ifndef BGMLIB_LIBVORBIS_H
#define BGMLIB_LIBVORBIS_H

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

extern const int OV_BLOCK;	// Ogg Vorbis read block size;

// Virtual file. Basically just a normal buffer with a read cursor.
// ------------
class VFile
{
public:
	char* Buf;
	ulong Size;
	volatile FXulong Read;	// Read cursor
	volatile FXulong Write;	// Write cursor

	VFile();
	VFile(const ulong& Size);
	~VFile();

	void Create(const ulong& Size);
	void Clear();
};
// ------------

class PackMethod;

/*// Encrypted archive file stream info structure.
// Provides streaming functionality and limits it to the region of the specified track
//   (with emphasis on the latter, because that's why it wasn't working and I seemed to need dump files in the first place!)
// Unfortunately, it wasn't as fast as one would have hoped, in fact, it was multiple orders of magnitude slower...
// ----------------------
struct CryptFile
{
public:
	FXFile& In;
	TrackInfo* TI;
	PackMethod* PM;	// reduces ugliness

	CryptFile(FXFile& _In, TrackInfo* _TI, PackMethod* _PM)	: In(_In), TI(_TI), PM(_PM)	{}
};
// ----------------------*/

// Custom callbacks
// ----------------
size_t FXFile_read(void* _DstBuf, size_t _ElementSize, size_t _Count, FXFile* _File);
int FXFile_seek(FXFile* _File, ogg_int64_t off, int whence);
int FXFile_close(FXFile* _File);
int FXFile_tell(FXFile* _File);

/*size_t CryptFile_read(char* _DstBuf, size_t _ElementSize, size_t _Count, CryptFile* _File);
int CryptFile_seek(CryptFile* _File, ogg_int64_t off, int whence);
int CryptFile_close(CryptFile* _File);
int CryptFile_tell(CryptFile* _File);*/

size_t VFile_read(void* _DstBuf, size_t _ElementSize, size_t _Count, VFile* _File);
int VFile_seek(VFile* _File, ogg_int64_t off, int whence);
int VFile_tell(VFile* _File);

static ov_callbacks OV_CALLBACKS_FXFILE =
{
	(size_t (*)(void*, size_t, size_t, void*)) FXFile_read,
	(int (*)(void*, ogg_int64_t, int))         FXFile_seek,
	(int (*)(void*))                           FXFile_close,
	(long (*)(void*))                          FXFile_tell
};

/*static ov_callbacks OV_CALLBACKS_CRYPTFILE =
{
	(size_t (*)(void*, size_t, size_t, void*)) CryptFile_read,
	(int (*)(void*, ogg_int64_t, int))         CryptFile_seek,
	(int (*)(void*))                           NULL,
	(long (*)(void*))                          CryptFile_tell
};*/

static ov_callbacks OV_CALLBACKS_VFILE =
{
	(size_t (*)(void*, size_t, size_t, void*)) VFile_read,
	(int (*)(void*, ogg_int64_t, int))         VFile_seek,	
	(int (*)(void*))                           NULL,
	(long (*)(void*))                          VFile_tell
};
// ----------------

bool DumpDecrypt(GameInfo* GI, TrackInfo* TI, const FXString& FN);
bool OpenVorbisBGM(FXFile& File, OggVorbis_File& VF, GameInfo* GI, TrackInfo* TI);	// Opens [GI->BGMFile], writes handles to [File] and [VF], and seeks to [TI]

// Encoding state
// --------------
struct OggVorbis_EncState
{
	FXFile*	out;
	ogg_stream_state stream_out; // collects Ogg packets and streams out pages
	vorbis_info      vi; // stores all the static vorbis bitstream settings
	vorbis_dsp_state vd; // central working state for the packet->PCM decoder
	vorbis_block     vb; // local working space for packet->PCM decode

	bool setup(FXFile* out, const float& freq, const float& quality);	// initializes the Ogg Vorbis structures

	bool write_headers();
	bool write_headers(vorbis_comment* vc);

	bool encode_pcm(char* buf, const int& size);
	// Encodes [bytes] bytes from [in]
	bool encode_file(FXFile& in, const ulong& bytes, char* buf, const ulong& bufsize);
	bool encode_file(FXFile& in, const ulong& bytes, char* buf, const ulong& bufsize, volatile FXulong& d, volatile bool* StopReq);

	void clear();

	OggVorbis_EncState();
	~OggVorbis_EncState();
};
// --------------

// Decodes [Size] bytes from [vf] into [buffer]. Loops according to the info in [TI].
ogg_int64_t ov_read_bgm(OggVorbis_File* vf, char* buffer, const ulong& Size, TrackInfo* TI);

// Ogg packet copy functions
// ===============
int ov_bitstream_seek(OggVorbis_File* vf, ogg_int64_t pos, bool seek_to_header);
void vorbis_write_headers(FXFile& out, ogg_stream_state* os, vorbis_dsp_state* vd, vorbis_comment* vc);
void vorbis_write_headers(FXFile& out, ogg_stream_state* os, vorbis_info* vi, vorbis_comment* vc);
void vorbis_write_headers(FXFile& out, ogg_stream_state* os, vorbis_info* vi, ogg_packet* header, vorbis_comment* vc, ogg_packet* header_code);

// Adapted from vcut.c
// -------

// Reads an arbitrary amount of bytes from [file_in] into [sync_in].
// Return value: number of read bytes
int ogg_update_sync(FXFile& file_in, ogg_sync_state* sync_in);

// Writes pages to the given file, or discards them if file is NULL.
bool ogg_write_pages_to_file(ogg_stream_state *stream, FXFile& file, bool flush);

// Submits [packet] to [stream_out], and writes filled pages to [out]. 
bool ogg_write_packet(FXFile& out, ogg_stream_state* stream_out, ogg_packet *packet);

// Copies audio packets from [file_in] to [file_out].
// Stops once a given number of samples, or the end of the input stream is reached
// Returns number of written samples
ogg_int64_t ogg_packetcopy(FXFile& file_out, ogg_stream_state* stream_out, FXFile& file_in, ogg_stream_state* stream_in, ogg_sync_state* sync_in, vorbis_info* info_in = NULL, ogg_int64_t samples = -1, ogg_int64_t sample_start = 0);
ogg_int64_t ogg_packetcopy(FXFile& file_out, ogg_stream_state* stream_out, OggVorbis_File* ov_in, ogg_int64_t sample_end = -1, ogg_int64_t sample_start = 0);

// Maps a Vorbis quality level to an average bitrate
float vorbis_quality_to_bitrate(const float& q);
// -------

// ===============

#endif /* BGMLIB_LIBVORBIS_H */
