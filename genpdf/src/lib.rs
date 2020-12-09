pub mod img2pdf;
pub mod png;

use libc::{c_int, c_uchar};

#[no_mangle]
pub extern "C" fn img_vec2pdf(input: *mut c_uchar, input_len: c_int, sizes: *mut c_uchar, sizes_len: c_int, output: *mut *mut c_uchar) -> c_int {
    let mut pdf = img2pdf::Pdf::new();

    let src = unsafe {
        Vec::from_raw_parts(input, input_len as usize, input_len as usize)
    };

    let sizes = unsafe {
        String::from_raw_parts(sizes, sizes_len as usize - 1, sizes_len as usize - 1)
    };

    let mut src_deref = &src[..];

    for size in sizes.lines() {
        let (target, left) = src_deref.split_at(size.parse().unwrap());
        pdf.add_image(target).unwrap();
        src_deref = left;
    }

    let mut result = Vec::new();
    pdf.save(&mut result).unwrap();
    result.shrink_to_fit();

    let result_size = result.len();
    unsafe { *output = result.as_mut_ptr() };

    std::mem::forget(result);
    std::mem::forget(src);
    std::mem::forget(sizes);

    result_size as c_int
}

#[no_mangle]
pub unsafe extern "C" fn dealloc(ptr: *mut *mut c_uchar, len: c_int) {
    drop(Vec::from_raw_parts(*ptr, len as usize, len as usize))
}
