#define threadBlockSize 32
#define dstBufferStride 8704
#define PXY_INNER_MAX_IFRAME_BATCH_COUNT 4096*2

RWByteAddressBuffer dstBuffer : register(u0); 
RWByteAddressBuffer srcBuffer : register(u1);

[numthreads(threadBlockSize, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
	uint2 shift = {dstBufferStride, 1};
	uint index = ((groupId.x * threadBlockSize) + groupIndex);

	//fv0
	//fv1
	//fv2
	//fv3		
	//dstOffset = x
	//startBatch = y 
	//endBatch = z
	//cacheAlignPlace
		
	index *= 16;	
	
	uint4 control = srcBuffer.Load4(index+PXY_INNER_MAX_IFRAME_BATCH_COUNT*dstBufferStride);
					
	if (control.z == 0)
		return;
		
	uint4 data = srcBuffer.Load4(index);		
	
	uint orgCtl = control.x;
	
	control.x *= 16;		
	control.x += control.y * dstBufferStride;
	
	index += 16 * control.z;
		
	uint4 ctlLookup = srcBuffer.Load4(index+PXY_INNER_MAX_IFRAME_BATCH_COUNT*dstBufferStride);
		
	while (ctlLookup.z != 0)
	{
		
		if ((ctlLookup.x <= orgCtl) && ((ctlLookup.x + ctlLookup.z) > orgCtl))
			break;
			
		index += 16 * ctlLookup.z;
		
		ctlLookup = srcBuffer.Load4(index+PXY_INNER_MAX_IFRAME_BATCH_COUNT*dstBufferStride);						
	}
	
	control.z = ctlLookup.y;
		
	while (control.y != control.z)
	{
		dstBuffer.Store4(control.x, data);
		control.xy += shift;
	}
}